#include <test/jtx/Account.h>
#include <test/jtx/Env.h>
#include <test/jtx/TestHelpers.h>
#include <test/jtx/amount.h>
#include <test/jtx/mpt.h>
#include <test/jtx/sig.h>
#include <test/jtx/ter.h>
#include <test/jtx/vault.h>

#include <xrpl/basics/Buffer.h>
#include <xrpl/basics/Number.h>
#include <xrpl/basics/Slice.h>
#include <xrpl/basics/base_uint.h>
#include <xrpl/basics/strHex.h>
#include <xrpl/beast/unit_test/suite.h>
#include <xrpl/beast/utility/Zero.h>
#include <xrpl/json/json_value.h>
#include <xrpl/protocol/AccountID.h>
#include <xrpl/protocol/ConfidentialTransfer.h>
#include <xrpl/protocol/Feature.h>
#include <xrpl/protocol/Indexes.h>
#include <xrpl/protocol/LedgerFormats.h>
#include <xrpl/protocol/Protocol.h>
#include <xrpl/protocol/SField.h>
#include <xrpl/protocol/STAmount.h>
#include <xrpl/protocol/STLedgerEntry.h>
#include <xrpl/protocol/SeqProxy.h>
#include <xrpl/protocol/Serializer.h>
#include <xrpl/protocol/TER.h>
#include <xrpl/protocol/TxFlags.h>
#include <xrpl/protocol/TxFormats.h>
#include <xrpl/protocol/UintTypes.h>
#include <xrpl/protocol/jss.h>

#include <utility/mpt_utility.h>

#include <secp256k1.h>
#include <secp256k1_mpt.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace xrpl::test {

namespace {

// Provers for the confidential lending proofs, built from mpt-crypto's
// low-level functions. On failure they return a zero-filled proof of the right
// size, so a test can submit it and expect tecBAD_PROOF.

secp256k1_pubkey
parseKey(Slice const& key)
{
    secp256k1_pubkey point;
    if (secp256k1_ec_pubkey_parse(mpt_secp256k1_context(), &point, key.data(), key.size()) != 1)
        Throw<std::runtime_error>("invalid point");
    return point;
}

std::pair<secp256k1_pubkey, secp256k1_pubkey>
parseCiphertext(Slice const& ciphertext)
{
    std::pair<secp256k1_pubkey, secp256k1_pubkey> pair;
    if (!mpt_make_ec_pair(ciphertext.data(), &pair.first, &pair.second))
        Throw<std::runtime_error>("invalid ciphertext");
    return pair;
}

Buffer
scalarSub(Buffer const& a, Buffer const& b)
{
    Buffer negated(kEcBlindingFactorLength);
    secp256k1_mpt_scalar_negate(negated.data(), b.data());
    Buffer result(kEcBlindingFactorLength);
    secp256k1_mpt_scalar_add(result.data(), a.data(), negated.data());
    return result;
}

Buffer
scalarMul(Buffer const& a, std::uint64_t n)
{
    Buffer scalar(kEcBlindingFactorLength);
    std::memset(scalar.data(), 0, scalar.size());
    for (int i = 0; i < 8; ++i)
        scalar.data()[31 - i] = static_cast<std::uint8_t>(n >> (8 * i));
    Buffer result(kEcBlindingFactorLength);
    secp256k1_mpt_scalar_mul(result.data(), a.data(), scalar.data());
    return result;
}

// What a sigma proof needs to prove ownership of a confidential balance
struct BalanceWitness
{
    Buffer privKey;
    Buffer pubKey;
    std::uint64_t balance;
    Buffer ciphertext;
    Buffer rho;
    Buffer commitment;
};

// XLS-96 Send sigma proof over any number of recipients sharing randomness r
Buffer
sigmaProof(
    BalanceWitness const& w,
    std::uint64_t amount,
    Buffer const& r,
    std::vector<std::pair<Buffer, Buffer>> const& recipients,
    Buffer const& commitment,
    UInt256 const& contextHash)
{
    auto const c1 = parseCiphertext(recipients.front().second).first;
    std::vector<secp256k1_pubkey> c2;
    std::vector<secp256k1_pubkey> keys;
    for (auto const& [key, ciphertext] : recipients)
    {
        c2.push_back(parseCiphertext(ciphertext).second);
        keys.push_back(parseKey(key));
    }
    auto const pcM = parseKey(commitment);
    auto const pkA = parseKey(w.pubKey);
    auto const pcB = parseKey(w.commitment);
    auto const [b1, b2] = parseCiphertext(w.ciphertext);

    Buffer proof(kEcSendSigmaProofLength);
    std::memset(proof.data(), 0, proof.size());
    secp256k1_compact_standard_prove(
        mpt_secp256k1_context(),
        proof.data(),
        amount,
        w.balance,
        r.data(),
        w.privKey.data(),
        w.rho.data(),
        recipients.size(),
        &c1,
        c2.data(),
        keys.data(),
        &pcM,
        &pkA,
        &pcB,
        &b1,
        &b2,
        contextHash.data());
    return proof;
}

// XLS-96 ConvertBack sigma proof: commitment commits to the value of
// ciphertext, which the holder of privKey can decrypt
Buffer
linkProof(
    Buffer const& privKey,
    Buffer const& pubKey,
    std::uint64_t value,
    Buffer const& rho,
    Slice const& ciphertext,
    Buffer const& commitment,
    UInt256 const& contextHash)
{
    auto const pk = parseKey(pubKey);
    auto const pc = parseKey(commitment);
    auto const [b1, b2] = parseCiphertext(ciphertext);
    Buffer proof(kEcConvertBackSigmaProofLength);
    std::memset(proof.data(), 0, proof.size());
    secp256k1_compact_convertback_prove(
        mpt_secp256k1_context(),
        proof.data(),
        value,
        privKey.data(),
        rho.data(),
        &pk,
        &b1,
        &b2,
        &pc,
        contextHash.data());
    return proof;
}

// Aggregated Bulletproof that each value is in [0, 2^64)
Buffer
rangeProof(
    std::vector<std::uint64_t> const& values,
    std::vector<Buffer> const& blindings,
    UInt256 const& contextHash)
{
    std::vector<std::uint8_t> flat;
    for (auto const& b : blindings)
        flat.insert(flat.end(), b.data(), b.data() + b.size());
    secp256k1_pubkey h;
    secp256k1_mpt_get_h_generator(mpt_secp256k1_context(), &h);

    Buffer proof(kEcDoubleBulletproofLength);
    std::memset(proof.data(), 0, proof.size());
    std::size_t length = proof.size();
    secp256k1_bulletproof_prove_agg(
        mpt_secp256k1_context(),
        proof.data(),
        &length,
        values.data(),
        flat.data(),
        values.size(),
        &h,
        contextHash.data());
    return proof;
}

// XLS-96 Clawback sigma proof with amount 0: ciphertext encrypts 0
Buffer
zeroProof(
    Buffer const& privKey,
    Buffer const& pubKey,
    Slice const& ciphertext,
    UInt256 const& contextHash)
{
    auto const pk = parseKey(pubKey);
    auto const [c1, c2] = parseCiphertext(ciphertext);
    Buffer proof(kEcClawbackProofLength);
    std::memset(proof.data(), 0, proof.size());
    secp256k1_compact_clawback_prove(
        mpt_secp256k1_context(),
        proof.data(),
        0,
        privKey.data(),
        &pk,
        &c1,
        &c2,
        contextHash.data());
    return proof;
}

Buffer
concat(std::vector<Buffer> const& parts)
{
    std::size_t size = 0;
    for (auto const& p : parts)
        size += p.size();
    Buffer out(size);
    std::size_t offset = 0;
    for (auto const& p : parts)
    {
        std::memcpy(out.data() + offset, p.data(), p.size());
        offset += p.size();
    }
    return out;
}

Buffer
toBuffer(Slice const& s)
{
    return Buffer{s.data(), s.size()};
}

}  // namespace

class ConfidentialLoanBroker_test : public beast::unit_test::Suite
{
    // LendingProtocolV1_2 is excluded so that new Vaults are CashBasis, which
    // is the only Vault version a confidential LoanBroker supports.
    FeatureBitset const all_{jtx::testableAmendments() - featureLendingProtocolV1_2};

    // A confidential MPT with issuer and auditor keys, a funded closed-ended
    // Vault owned by the lender, and an ElGamal key pair for the Broker Key.
    struct Setup
    {
        jtx::Env& env;
        jtx::Account const issuer{"issuer"};
        jtx::Account const lender{"lender"};
        jtx::Account const auditor{"auditor"};
        // Holds the Broker Key pair; never submits a transaction.
        jtx::Account const brokerKey{"brokerKey"};
        // Holds a confidential balance and borrows from the broker.
        jtx::Account const borrower{"borrower"};
        // Holds the Loan Auditor key pair; never submits a transaction.
        jtx::Account const loanAuditor{"loanAuditor"};
        jtx::MPTTester mpt;
        Keylet vaultKeylet = keylet::vault(beast::kZero);

        Setup(
            jtx::Env& env_,
            bool investment = true,
            std::uint32_t mptFlags = tfMPTCanTransfer | tfMPTCanLock |
                tfMPTCanHoldConfidentialBalance,
            bool withAuditor = true)
            : env(env_), mpt(env, issuer, {.holders = {lender, borrower}, .auditor = auditor})
        {
            using namespace jtx;

            mpt.create({.ownerCount = 1, .flags = mptFlags});
            mpt.authorize({.account = lender});
            mpt.pay(issuer, lender, 10'000);
            mpt.authorize({.account = borrower});
            mpt.pay(issuer, borrower, 2'000);

            mpt.generateKeyPair(issuer);
            mpt.generateKeyPair(auditor);
            mpt.generateKeyPair(brokerKey);
            if (mptFlags & tfMPTCanHoldConfidentialBalance)
            {
                mpt.set({
                    .account = issuer,
                    .issuerPubKey = mpt.getPubKey(issuer),
                    .auditorPubKey =
                        withAuditor ? mpt.getPubKey(auditor) : std::optional<Buffer>{},
                });

                // The Borrower holds a confidential spending balance of 2000
                if (withAuditor)
                {
                    mpt.generateKeyPair(borrower);
                    mpt.generateKeyPair(loanAuditor);
                    mpt.convert(
                        {.account = borrower, .amt = 2'000, .holderPubKey = mpt.getPubKey(borrower)});
                    mpt.mergeInbox({.account = borrower});
                }
            }

            Vault const vault{env};
            // Without investment, leave a Subscription window that outlasts
            // the ledger closes the test makes.
            auto [tx, keylet, subscriptionDate] = vault.createClosedEnded({
                .owner = lender,
                .asset = mpt,
                .subscriptionOffset = std::chrono::seconds{investment ? 10 : 3'600},
            });
            env(tx);
            env.close();
            vaultKeylet = keylet;

            env(vault.deposit({.depositor = lender, .id = keylet.key, .amount = mpt(5'000)}));
            env.close();

            if (investment)
                vault.closePastSubscription(subscriptionDate);
        }

        // A confidential LoanBrokerSet for the given sequence
        [[nodiscard]] json::Value
        brokerSet(std::uint32_t seq) const
        {
            auto jv = jtx::loan_broker::set(lender.id(), vaultKeylet.key, tfLoanBrokerConfidential);
            jv[sfHolderEncryptionKey] = strHex(*mpt.getPubKey(brokerKey));
            auto const proof = mpt.getSchnorrProof(
                brokerKey, getLoanBrokerSetContextHash(lender.id(), mpt.issuanceID(), seq));
            jv[sfZKProof] = strHex(*proof);
            // jtx only autofills the confidential fee for XLS-96 transactions
            jv[jss::Fee] = to_string(env.current()->fees().base * (kConfidentialFeeMultiplier + 1));
            return jv;
        }

        Keylet
        createBroker()
        {
            auto const seq = env.seq(lender);
            env(brokerSet(seq));
            env.close();
            return keylet::loanBroker(lender.id(), SeqProxy::rawSequence(seq));
        }

        Keylet
        createPublicBroker()
        {
            auto const seq = env.seq(lender);
            env(jtx::loan_broker::set(lender.id(), vaultKeylet.key));
            env.close();
            return keylet::loanBroker(lender.id(), SeqProxy::rawSequence(seq));
        }

        // Ciphertexts encrypting amt under each key, with one blinding factor
        [[nodiscard]] json::Value
        draw(uint256 const& brokerID, std::uint64_t amt) const
        {
            auto const bf = generateBlindingFactor();
            json::Value jv;
            jv[sfTransactionType] = jss::LoanBrokerDraw;
            jv[sfAccount] = to_string(lender.id());
            jv[sfLoanBrokerID] = to_string(brokerID);
            jv[sfAmount] = mpt(amt).value().getJson(JsonOptions::Values::None);
            jv[sfHolderEncryptedAmount] = strHex(mpt.encryptAmount(brokerKey, amt, bf));
            jv[sfIssuerEncryptedAmount] = strHex(mpt.encryptAmount(issuer, amt, bf));
            jv[sfAuditorEncryptedAmount] = strHex(mpt.encryptAmount(auditor, amt, bf));
            jv[sfBlindingFactor] = strHex(bf);
            return jv;
        }

        [[nodiscard]] json::Value
        mergeInbox(uint256 const& brokerID) const
        {
            json::Value jv;
            jv[sfTransactionType] = jss::LoanBrokerMergeInbox;
            jv[sfAccount] = to_string(lender.id());
            jv[sfLoanBrokerID] = to_string(brokerID);
            return jv;
        }

        [[nodiscard]] AccountID
        pseudo(Keylet const& broker) const
        {
            return env.le(broker)->at(sfAccount);
        }

        [[nodiscard]] SLE::const_pointer
        lending(Keylet const& broker) const
        {
            return env.le(keylet::mptoken(mpt.issuanceID(), pseudo(broker)));
        }

        // Decrypt one of the Lending Balance's ciphertexts with key's secret key
        [[nodiscard]] std::optional<std::uint64_t>
        decrypt(Keylet const& broker, SF_VL const& field, jtx::Account const& key) const
        {
            auto const sle = lending(broker);
            if (!sle || !sle->isFieldPresent(field))
                return std::nullopt;
            auto const ciphertext = (*sle)[field];
            return mpt.decryptAmount(key, Buffer(ciphertext.data(), ciphertext.size()));
        }

        [[nodiscard]] Number
        vaultField(SF_NUMBER const& field) const
        {
            return env.le(vaultKeylet)->at(field);
        }

        [[nodiscard]] Buffer
        pubKey(jtx::Account const& a) const
        {
            return *mpt.getPubKey(a);
        }

        // The witness for a confidential spending balance held under key's
        // key pair
        [[nodiscard]] BalanceWitness
        witness(jtx::Account const& key, SLE const& token) const
        {
            auto const spending = toBuffer(token[sfConfidentialBalanceSpending]);
            auto const balance = mpt.decryptAmount(key, spending).value_or(0);
            auto rho = generateBlindingFactor();
            auto commitment = jtx::MPTTester::getPedersenCommitment(balance, rho);
            return {
                *mpt.getPrivKey(key),
                pubKey(key),
                balance,
                spending,
                std::move(rho),
                std::move(commitment)};
        }

        struct LoanTerms
        {
            std::uint64_t principal;
            std::uint64_t owed;
            std::uint64_t periodic;
            std::uint32_t paymentTotal = 12;
            std::uint32_t paymentInterval = 600;
        };

        // A confidential LoanSet submitted by the lender, with the full
        // spec §11.2 proof. The Borrower must add CounterpartySignature.
        [[nodiscard]] json::Value
        loanSet(Keylet const& broker, LoanTerms const& t) const
        {
            auto const seq = env.seq(lender);
            auto const rP = generateBlindingFactor();
            auto const rO = generateBlindingFactor();
            auto const rQ = generateBlindingFactor();
            auto const enc = [this](jtx::Account const& a, std::uint64_t v, Buffer const& r) {
                return mpt.encryptAmount(a, v, r);
            };

            auto const senderP = enc(brokerKey, t.principal, rP);
            auto const destP = enc(borrower, t.principal, rP);
            auto const issuerP = enc(issuer, t.principal, rP);
            auto const auditorP = enc(auditor, t.principal, rP);
            auto const loanAuditorP = enc(loanAuditor, t.principal, rP);
            auto const borrowerO = enc(borrower, t.owed, rO);
            auto const brokerO = enc(brokerKey, t.owed, rO);
            auto const loanAuditorO = enc(loanAuditor, t.owed, rO);
            auto const borrowerQ = enc(borrower, t.periodic, rQ);
            auto const brokerQ = enc(brokerKey, t.periodic, rQ);
            auto const loanAuditorQ = enc(loanAuditor, t.periodic, rQ);
            auto const pcP = jtx::MPTTester::getPedersenCommitment(t.principal, rP);
            auto const pcO = jtx::MPTTester::getPedersenCommitment(t.owed, rO);
            auto const pcQ = jtx::MPTTester::getPedersenCommitment(t.periodic, rQ);

            auto const lendingSle = lending(broker);
            auto const w = witness(brokerKey, *lendingSle);
            auto const contextHash = getLoanSetContextHash(
                pseudo(broker),
                mpt.issuanceID(),
                seq,
                borrower.id(),
                (*lendingSle)[~sfConfidentialBalanceVersion].value_or(0));

            auto const send = mpt.getConfidentialSendProof(
                brokerKey,
                t.principal,
                {{.publicKey = pubKey(brokerKey), .encryptedAmount = senderP},
                 {.publicKey = pubKey(borrower), .encryptedAmount = destP},
                 {.publicKey = pubKey(issuer), .encryptedAmount = issuerP},
                 {.publicKey = pubKey(auditor), .encryptedAmount = auditorP}},
                rP,
                contextHash,
                {.pedersenCommitment = pcP, .amt = t.principal, .encryptedAmt = senderP, .blindingFactor = rP},
                {.pedersenCommitment = w.commitment,
                 .amt = w.balance,
                 .encryptedAmt = w.ciphertext,
                 .blindingFactor = w.rho});
            auto const proof = concat({
                send.value_or(Buffer(kEcSendProofLength)),
                sigmaProof(w, t.principal, rP, {{pubKey(loanAuditor), loanAuditorP}}, pcP, contextHash),
                sigmaProof(
                    w,
                    t.owed,
                    rO,
                    {{pubKey(borrower), borrowerO},
                     {pubKey(brokerKey), brokerO},
                     {pubKey(loanAuditor), loanAuditorO}},
                    pcO,
                    contextHash),
                sigmaProof(
                    w,
                    t.periodic,
                    rQ,
                    {{pubKey(borrower), borrowerQ},
                     {pubKey(brokerKey), brokerQ},
                     {pubKey(loanAuditor), loanAuditorQ}},
                    pcQ,
                    contextHash),
                rangeProof({t.owed, t.periodic}, {rO, rQ}, contextHash),
                rangeProof(
                    {t.owed - t.principal, (t.paymentTotal * t.periodic) - t.owed},
                    {scalarSub(rO, rP), scalarSub(scalarMul(rQ, t.paymentTotal), rO)},
                    contextHash),
            });

            json::Value jv;
            jv[sfTransactionType] = jss::LoanSet;
            jv[sfAccount] = to_string(lender.id());
            jv[sfLoanBrokerID] = to_string(broker.key);
            jv[sfCounterparty] = to_string(borrower.id());
            jv[sfPaymentTotal] = t.paymentTotal;
            jv[sfPaymentInterval] = t.paymentInterval;
            jv[sfSenderEncryptedAmount] = strHex(senderP);
            jv[sfDestinationEncryptedAmount] = strHex(destP);
            jv[sfIssuerEncryptedAmount] = strHex(issuerP);
            jv[sfAuditorEncryptedAmount] = strHex(auditorP);
            jv[sfBalanceCommitment] = strHex(w.commitment);
            jv[sfAmountCommitment] = strHex(pcP);
            jv[sfZKProof] = strHex(proof);
            jv[sfLoanAuditorEncryptionKey] = strHex(pubKey(loanAuditor));
            jv[sfLoanAuditorEncryptedPrincipal] = strHex(loanAuditorP);
            jv[sfBorrowerEncryptedAmountOwed] = strHex(borrowerO);
            jv[sfBrokerEncryptedAmountOwed] = strHex(brokerO);
            jv[sfLoanAuditorEncryptedAmountOwed] = strHex(loanAuditorO);
            jv[sfBorrowerEncryptedPeriodicPayment] = strHex(borrowerQ);
            jv[sfBrokerEncryptedPeriodicPayment] = strHex(brokerQ);
            jv[sfLoanAuditorEncryptedPeriodicPayment] = strHex(loanAuditorQ);
            jv[sfAmountOwedCommitment] = strHex(pcO);
            jv[sfPeriodicPaymentCommitment] = strHex(pcQ);
            // Base fee, one counterparty signature and the confidential multiplier
            jv[jss::Fee] = to_string(env.current()->fees().base * (kConfidentialFeeMultiplier + 2));
            return jv;
        }

        // A ConfidentialLoanPay of amt, with the spec §13.3 proof
        [[nodiscard]] json::Value
        loanPay(Keylet const& loanKeylet, Keylet const& broker, std::uint64_t amt, bool full) const
        {
            auto const seq = env.seq(borrower);
            auto const r = generateBlindingFactor();
            auto const enc = [&](jtx::Account const& a) { return mpt.encryptAmount(a, amt, r); };
            auto const senderX = enc(borrower);
            auto const destX = enc(brokerKey);
            auto const issuerX = enc(issuer);
            auto const auditorX = enc(auditor);
            auto const loanAuditorX = enc(loanAuditor);
            auto const pcX = jtx::MPTTester::getPedersenCommitment(amt, r);

            auto const loan = env.le(loanKeylet);
            auto const token = env.le(keylet::mptoken(mpt.issuanceID(), borrower.id()));
            auto const w = witness(borrower, *token);
            auto const contextHash = getConfidentialLoanPayContextHash(
                borrower.id(),
                mpt.issuanceID(),
                seq,
                pseudo(broker),
                (*token)[~sfConfidentialBalanceVersion].value_or(0),
                loanKeylet.key,
                loan->at(~sfConfidentialLoanVersion).value_or(0));

            auto const send = mpt.getConfidentialSendProof(
                borrower,
                amt,
                {{.publicKey = pubKey(borrower), .encryptedAmount = senderX},
                 {.publicKey = pubKey(brokerKey), .encryptedAmount = destX},
                 {.publicKey = pubKey(issuer), .encryptedAmount = issuerX},
                 {.publicKey = pubKey(auditor), .encryptedAmount = auditorX}},
                r,
                contextHash,
                {.pedersenCommitment = pcX, .amt = amt, .encryptedAmt = senderX, .blindingFactor = r},
                {.pedersenCommitment = w.commitment,
                 .amt = w.balance,
                 .encryptedAmt = w.ciphertext,
                 .blindingFactor = w.rho});
            std::vector<Buffer> parts{
                send.value_or(Buffer(kEcSendProofLength)),
                sigmaProof(w, amt, r, {{pubKey(loanAuditor), loanAuditorX}}, pcX, contextHash)};

            json::Value jv;
            auto const owedCt = toBuffer(loan->at(sfBorrowerEncryptedAmountOwed));
            if (full)
            {
                auto const difference = homomorphicSubtract(senderX, owedCt);
                parts.push_back(zeroProof(
                    w.privKey,
                    w.pubKey,
                    difference ? Slice(*difference) : Slice(owedCt),
                    contextHash));
                jv[sfFlags] = tfLoanFullPayment;
            }
            else
            {
                auto const periodicCt = toBuffer(loan->at(sfBorrowerEncryptedPeriodicPayment));
                auto const owed = mpt.decryptAmount(borrower, owedCt).value_or(0);
                auto const periodic = mpt.decryptAmount(borrower, periodicCt).value_or(0);
                auto const rhoO = generateBlindingFactor();
                auto const rhoQ = generateBlindingFactor();
                auto const pcO = jtx::MPTTester::getPedersenCommitment(owed, rhoO);
                auto const pcQ = jtx::MPTTester::getPedersenCommitment(periodic, rhoQ);
                parts.push_back(linkProof(w.privKey, w.pubKey, owed, rhoO, owedCt, pcO, contextHash));
                parts.push_back(
                    linkProof(w.privKey, w.pubKey, periodic, rhoQ, periodicCt, pcQ, contextHash));
                parts.push_back(rangeProof(
                    {amt - periodic, owed - amt}, {scalarSub(r, rhoQ), scalarSub(rhoO, r)}, contextHash));
                jv[sfAmountOwedCommitment] = strHex(pcO);
                jv[sfPeriodicPaymentCommitment] = strHex(pcQ);
            }

            jv[sfTransactionType] = jss::ConfidentialLoanPay;
            jv[sfAccount] = to_string(borrower.id());
            jv[sfLoanID] = to_string(loanKeylet.key);
            jv[sfSenderEncryptedAmount] = strHex(senderX);
            jv[sfDestinationEncryptedAmount] = strHex(destX);
            jv[sfIssuerEncryptedAmount] = strHex(issuerX);
            jv[sfAuditorEncryptedAmount] = strHex(auditorX);
            jv[sfLoanAuditorEncryptedAmount] = strHex(loanAuditorX);
            jv[sfBalanceCommitment] = strHex(w.commitment);
            jv[sfAmountCommitment] = strHex(pcX);
            jv[sfZKProof] = strHex(concat(parts));
            jv[jss::Fee] = to_string(env.current()->fees().base * (kConfidentialFeeMultiplier + 1));
            return jv;
        }

        // A LoanBrokerSettle of amt, with the XLS-96 ConvertBack proof
        [[nodiscard]] json::Value
        settle(Keylet const& broker, std::uint64_t amt) const
        {
            auto const seq = env.seq(lender);
            auto const bf = generateBlindingFactor();
            auto const lendingSle = lending(broker);
            auto const w = witness(brokerKey, *lendingSle);
            auto const contextHash = getLoanBrokerSettleContextHash(
                pseudo(broker),
                mpt.issuanceID(),
                seq,
                (*lendingSle)[~sfConfidentialBalanceVersion].value_or(0));

            mpt_pedersen_proof_params params{};
            std::memcpy(params.pedersen_commitment, w.commitment.data(), w.commitment.size());
            params.amount = w.balance;
            std::memcpy(params.ciphertext, w.ciphertext.data(), w.ciphertext.size());
            std::memcpy(params.blinding_factor, w.rho.data(), w.rho.size());
            Buffer proof(kEcConvertBackProofLength);
            std::memset(proof.data(), 0, proof.size());
            mpt_get_convert_back_proof(
                w.privKey.data(), w.pubKey.data(), contextHash.data(), amt, &params, proof.data());

            json::Value jv;
            jv[sfTransactionType] = jss::LoanBrokerSettle;
            jv[sfAccount] = to_string(lender.id());
            jv[sfLoanBrokerID] = to_string(broker.key);
            jv[sfAmount] = mpt(amt).value().getJson(JsonOptions::Values::None);
            jv[sfHolderEncryptedAmount] = strHex(mpt.encryptAmount(brokerKey, amt, bf));
            jv[sfIssuerEncryptedAmount] = strHex(mpt.encryptAmount(issuer, amt, bf));
            jv[sfAuditorEncryptedAmount] = strHex(mpt.encryptAmount(auditor, amt, bf));
            jv[sfBlindingFactor] = strHex(bf);
            jv[sfBalanceCommitment] = strHex(w.commitment);
            jv[sfZKProof] = strHex(proof);
            jv[jss::Fee] = to_string(env.current()->fees().base * (kConfidentialFeeMultiplier + 1));
            return jv;
        }

        // Decrypt one of a Loan's encrypted terms with key's secret key
        [[nodiscard]] std::optional<std::uint64_t>
        decryptLoan(Keylet const& loan, SF_VL const& field, jtx::Account const& key) const
        {
            auto const sle = env.le(loan);
            if (!sle || !sle->isFieldPresent(field))
                return std::nullopt;
            return mpt.decryptAmount(key, toBuffer((*sle)[field]));
        }
    };

    void
    testContextHash()
    {
        testcase("Context hash");

        // getLoanBrokerSetContextHash uses the same layout as the
        // ConfidentialMPTConvert context hash from mpt-crypto, with only the
        // transaction type changed. Check that layout against mpt-crypto.
        AccountID const account = jtx::Account("alice").id();
        UInt192 const issuanceID = makeMptID(7, account);
        std::uint32_t const seq = 42;

        Serializer s;
        s.add16(ttCONFIDENTIAL_MPT_CONVERT);
        s.addBitString(account);
        s.addBitString(issuanceID);
        s.add32(seq);
        s.addBitString(account);
        s.add32(0);
        BEAST_EXPECT(s.getSHA512Half() == getConvertContextHash(account, issuanceID, seq));

        BEAST_EXPECT(
            getLoanBrokerSetContextHash(account, issuanceID, seq) !=
            getConvertContextHash(account, issuanceID, seq));
    }

    void
    testDisabled()
    {
        testcase("Disabled");
        using namespace jtx;

        Env env{*this, all_ - featureConfidentialLendingProtocol};
        Setup s{env};

        auto const seq = env.seq(s.lender);
        env(s.brokerSet(seq), Ter(temDISABLED));

        // The confidential fields are also gated without the flag
        auto jv = loan_broker::set(s.lender.id(), s.vaultKeylet.key);
        jv[sfHolderEncryptionKey] = strHex(*s.mpt.getPubKey(s.brokerKey));
        env(jv, Ter(temDISABLED));

        auto const broker = s.createPublicBroker();
        env(s.draw(broker.key, 100), Ter(temDISABLED));
        env(s.mergeInbox(broker.key), Ter(temDISABLED));
    }

    void
    testBrokerSet()
    {
        testcase("LoanBrokerSet");
        using namespace jtx;

        Env env{*this, all_};
        Setup s{env};

        // Malformed
        {
            auto const seq = env.seq(s.lender);

            auto noKey = s.brokerSet(seq);
            noKey.removeMember(sfHolderEncryptionKey.jsonName);
            env(noKey, Ter(temMALFORMED));

            auto noProof = s.brokerSet(seq);
            noProof.removeMember(sfZKProof.jsonName);
            env(noProof, Ter(temMALFORMED));

            auto keyWithoutFlag = s.brokerSet(seq);
            keyWithoutFlag[sfFlags] = 0;
            env(keyWithoutFlag, Ter(temMALFORMED));

            auto badKey = s.brokerSet(seq);
            badKey[sfHolderEncryptionKey] = strHex(std::string(33, '\0'));
            env(badKey, Ter(temMALFORMED));

            auto shortProof = s.brokerSet(seq);
            shortProof[sfZKProof] = strHex(std::string(10, '\x01'));
            env(shortProof, Ter(temMALFORMED));

            auto withFee = s.brokerSet(seq);
            withFee[sfManagementFeeRate] = 10;
            env(withFee, Ter(temINVALID));

            auto withCover = s.brokerSet(seq);
            withCover[sfCoverRateMinimum] = 10;
            withCover[sfCoverRateLiquidation] = 10;
            env(withCover, Ter(temINVALID));
        }

        // A confidential LoanBrokerSet costs 10 times the base fee
        {
            auto cheap = s.brokerSet(env.seq(s.lender));
            cheap[jss::Fee] = to_string(env.current()->fees().base);
            env(cheap, Ter(telINSUF_FEE_P));
        }

        // A proof bound to another sequence does not verify
        env(s.brokerSet(env.seq(s.lender) + 1), Ter(tecBAD_PROOF));
        env.close();

        auto const broker = s.createBroker();
        auto const sleBroker = env.le(broker);
        if (!BEAST_EXPECT(sleBroker))
            return;
        BEAST_EXPECT(sleBroker->isFlag(lsfLoanBrokerConfidential));

        // The Lending Balance is initialized to zero under every key
        auto const sleLending = s.lending(broker);
        if (!BEAST_EXPECT(sleLending))
            return;
        BEAST_EXPECT(
            (*sleLending)[sfHolderEncryptionKey] == Slice(*s.mpt.getPubKey(s.brokerKey)));
        BEAST_EXPECT((*sleLending)[sfConfidentialBalanceVersion] == 0);
        BEAST_EXPECT((*sleLending)[sfMPTAmount] == 0);
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceSpending, s.brokerKey) == 0);
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceInbox, s.brokerKey) == 0);
        BEAST_EXPECT(s.decrypt(broker, sfIssuerEncryptedBalance, s.issuer) == 0);
        BEAST_EXPECT(s.decrypt(broker, sfAuditorEncryptedBalance, s.auditor) == 0);

        // Both zeros are the canonical encrypted zero for the pseudo-account
        auto const zero =
            encryptCanonicalZeroAmount(*s.mpt.getPubKey(s.brokerKey), s.pseudo(broker), s.mpt.issuanceID());
        BEAST_EXPECT(zero && (*sleLending)[sfConfidentialBalanceSpending] == Slice(*zero));
        BEAST_EXPECT(zero && (*sleLending)[sfConfidentialBalanceInbox] == Slice(*zero));

        // A LoanBroker can only be made confidential when it is created
        {
            auto update = loan_broker::set(s.lender.id(), s.vaultKeylet.key, tfLoanBrokerConfidential);
            update[sfLoanBrokerID] = to_string(broker.key);
            env(update, Ter(temINVALID));
        }

        // An update of a confidential LoanBroker works as before
        {
            auto update = loan_broker::set(s.lender.id(), s.vaultKeylet.key);
            update[sfLoanBrokerID] = to_string(broker.key);
            update[sfDebtMaximum] = "1000";
            env(update);
            env.close();
            BEAST_EXPECT(env.le(broker)->isFlag(lsfLoanBrokerConfidential));
        }

        // A confidential LoanBroker has no First-Loss Capital
        env(loan_broker::coverDeposit(s.lender.id(), broker.key, s.mpt(10)),
            Ter(tecNO_PERMISSION));
    }

    void
    testBrokerSetIssuance()
    {
        testcase("LoanBrokerSet issuance requirements");
        using namespace jtx;

        auto const expectRejected = [&](std::uint32_t mptFlags, bool withAuditor) {
            Env env{*this, all_};
            Setup s{env, true, mptFlags, withAuditor};
            env(s.brokerSet(env.seq(s.lender)), Ter(tecNO_PERMISSION));
        };

        // No auditor key
        expectRejected(tfMPTCanTransfer | tfMPTCanLock | tfMPTCanHoldConfidentialBalance, false);
        // Cannot hold confidential balances
        expectRejected(tfMPTCanTransfer | tfMPTCanLock, true);

        // The Vault must be CashBasis, which LendingProtocolV1_2 Vaults are not
        {
            Env env{*this, all_ | featureLendingProtocolV1_2};
            Setup s{env};
            env(s.brokerSet(env.seq(s.lender)), Ter(tecNO_PERMISSION));
        }
    }

    void
    testDraw()
    {
        testcase("LoanBrokerDraw");
        using namespace jtx;

        Env env{*this, all_};
        Setup s{env};
        auto const broker = s.createBroker();
        auto const vaultPseudo = env.le(s.vaultKeylet)->at(sfAccount);

        // Malformed
        {
            auto zero = s.draw(broker.key, 0);
            env(zero, Ter(temBAD_AMOUNT));

            auto xrp = s.draw(broker.key, 100);
            xrp[sfAmount] = XRP(100).value().getJson(JsonOptions::Values::None);
            env(xrp, Ter(temBAD_AMOUNT));

            env(s.draw(beast::kZero, 100), Ter(temINVALID));

            auto badCiphertext = s.draw(broker.key, 100);
            badCiphertext[sfIssuerEncryptedAmount] = strHex(std::string(66, '\0'));
            env(badCiphertext, Ter(temBAD_CIPHERTEXT));
        }

        // Protocol failures
        env(s.draw(keylet::loanBroker(s.lender.id(), SeqProxy::rawSequence(1)).key, 100), Ter(tecNO_ENTRY));
        {
            auto notOwner = s.draw(broker.key, 100);
            notOwner[sfAccount] = to_string(s.issuer.id());
            env(notOwner, Ter(tecNO_PERMISSION));
        }
        env(s.draw(broker.key, 5'001), Ter(tecINSUFFICIENT_FUNDS));
        {
            // The ciphertexts must encrypt Amount with BlindingFactor
            auto wrongAmount = s.draw(broker.key, 100);
            wrongAmount[sfAmount] = s.mpt(101).value().getJson(JsonOptions::Values::None);
            env(wrongAmount, Ter(tecBAD_PROOF));

            auto wrongBlinding = s.draw(broker.key, 100);
            wrongBlinding[sfBlindingFactor] = strHex(generateBlindingFactor());
            env(wrongBlinding, Ter(tecBAD_PROOF));
        }

        auto const outstanding = s.mpt.getIssuanceOutstandingBalance();
        auto const vaultBalance = s.mpt.getBalance(Account("vault", vaultPseudo));

        env(s.draw(broker.key, 1'000));
        env.close();

        BEAST_EXPECT(s.vaultField(sfAssetsAvailable) == Number(4'000));
        BEAST_EXPECT(s.vaultField(sfAssetsTotal) == Number(5'000));
        BEAST_EXPECT(env.le(broker)->at(sfDebtTotal) == Number(1'000));
        BEAST_EXPECT(s.mpt.getBalance(Account("vault", vaultPseudo)) == vaultBalance - 1'000);
        // The Borrower's 2000 plus the 1000 drawn
        BEAST_EXPECT(s.mpt.getIssuanceConfidentialBalance() == 3'000);
        BEAST_EXPECT(s.mpt.getIssuanceOutstandingBalance() == outstanding);

        // Credited to the inbox, not the spending balance
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceInbox, s.brokerKey) == 1'000);
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceSpending, s.brokerKey) == 0);
        BEAST_EXPECT(s.decrypt(broker, sfIssuerEncryptedBalance, s.issuer) == 1'000);
        BEAST_EXPECT(s.decrypt(broker, sfAuditorEncryptedBalance, s.auditor) == 1'000);
        BEAST_EXPECT((*s.lending(broker))[sfConfidentialBalanceVersion] == 0);

        // Draws accumulate in the inbox
        env(s.draw(broker.key, 500));
        env.close();
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceInbox, s.brokerKey) == 1'500);
        BEAST_EXPECT(env.le(broker)->at(sfDebtTotal) == Number(1'500));

        // Locked pseudo-accounts cannot draw
        // (close after each draw, or the closed ledger would apply it after the unlock)
        s.mpt.set({.account = s.issuer, .holder = s.pseudo(broker), .flags = tfMPTLock});
        env(s.draw(broker.key, 1), Ter(tecLOCKED));
        env.close();
        s.mpt.set({.account = s.issuer, .holder = s.pseudo(broker), .flags = tfMPTUnlock});
        s.mpt.set({.account = s.issuer, .holder = vaultPseudo, .flags = tfMPTLock});
        env(s.draw(broker.key, 1), Ter(tecLOCKED));
        env.close();
        s.mpt.set({.account = s.issuer, .holder = vaultPseudo, .flags = tfMPTUnlock});
        BEAST_EXPECT(env.le(broker)->at(sfDebtTotal) == Number(1'500));

        // DebtMaximum limits DebtTotal
        {
            auto update = loan_broker::set(s.lender.id(), s.vaultKeylet.key);
            update[sfLoanBrokerID] = to_string(broker.key);
            update[sfDebtMaximum] = "2000";
            env(update);
            env.close();
        }
        env(s.draw(broker.key, 501), Ter(tecLIMIT_EXCEEDED));
        env(s.draw(broker.key, 500));
        env.close();

        // A public LoanBroker cannot draw
        auto const publicBroker = s.createPublicBroker();
        env(s.draw(publicBroker.key, 100), Ter(tecNO_PERMISSION));
    }

    void
    testDrawPhase()
    {
        testcase("LoanBrokerDraw phase");
        using namespace jtx;

        // Draws are only allowed during the Vault's Investment phase
        Env env{*this, all_};
        Setup s{env, false};
        auto const broker = s.createBroker();
        env(s.draw(broker.key, 100), Ter(tecNO_PERMISSION));
    }

    void
    testMergeInbox()
    {
        testcase("LoanBrokerMergeInbox");
        using namespace jtx;

        Env env{*this, all_};
        Setup s{env};
        auto const broker = s.createBroker();

        env(s.mergeInbox(beast::kZero), Ter(temINVALID));
        env(s.mergeInbox(keylet::loanBroker(s.lender.id(), SeqProxy::rawSequence(1)).key), Ter(tecNO_ENTRY));
        {
            auto notOwner = s.mergeInbox(broker.key);
            notOwner[sfAccount] = to_string(s.issuer.id());
            env(notOwner, Ter(tecNO_PERMISSION));
        }
        {
            auto const publicBroker = s.createPublicBroker();
            env(s.mergeInbox(publicBroker.key), Ter(tecNO_PERMISSION));
        }

        env(s.draw(broker.key, 1'000));
        env.close();

        auto const outstanding = s.mpt.getIssuanceOutstandingBalance();
        env(s.mergeInbox(broker.key));
        env.close();

        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceSpending, s.brokerKey) == 1'000);
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceInbox, s.brokerKey) == 0);
        BEAST_EXPECT(s.decrypt(broker, sfIssuerEncryptedBalance, s.issuer) == 1'000);
        BEAST_EXPECT(s.decrypt(broker, sfAuditorEncryptedBalance, s.auditor) == 1'000);
        BEAST_EXPECT((*s.lending(broker))[sfConfidentialBalanceVersion] == 1);
        // The Borrower's 2000 plus the 1000 drawn
        BEAST_EXPECT(s.mpt.getIssuanceConfidentialBalance() == 3'000);
        BEAST_EXPECT(s.mpt.getIssuanceOutstandingBalance() == outstanding);

        // The inbox is reset to the canonical encrypted zero
        auto const zero = encryptCanonicalZeroAmount(
            *s.mpt.getPubKey(s.brokerKey), s.pseudo(broker), s.mpt.issuanceID());
        BEAST_EXPECT(zero && (*s.lending(broker))[sfConfidentialBalanceInbox] == Slice(*zero));

        // Merging an empty inbox still succeeds and increments the version
        env(s.mergeInbox(broker.key));
        env.close();
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceSpending, s.brokerKey) == 1'000);
        BEAST_EXPECT((*s.lending(broker))[sfConfidentialBalanceVersion] == 2);

        // A locked pseudo-account cannot merge
        s.mpt.set({.account = s.issuer, .holder = s.pseudo(broker), .flags = tfMPTLock});
        env(s.mergeInbox(broker.key), Ter(tecLOCKED));
    }

    void
    testLoanLifecycle()
    {
        testcase("Confidential loan lifecycle");
        using namespace jtx;

        Env env{*this, all_};
        Setup s{env};
        auto const broker = s.createBroker();
        env(s.draw(broker.key, 1'000));
        env.close();
        env(s.mergeInbox(broker.key));
        env.close();

        Setup::LoanTerms const terms{.principal = 500, .owed = 550, .periodic = 50};

        // A tampered proof does not verify
        {
            auto bad = s.loanSet(broker, terms);
            auto proof = strUnHex(bad[sfZKProof].asString());
            (*proof)[kEcSendProofLength + 10] ^= 0x01;
            bad[sfZKProof] = strHex(*proof);
            env(bad, Sig(sfCounterpartySignature, s.borrower), Ter(tecBAD_PROOF));
            env.close();
        }

        // LoanSet: the principal moves from the Lending Balance to the Borrower
        env(s.loanSet(broker, terms), Sig(sfCounterpartySignature, s.borrower));
        env.close();
        auto const loan = keylet::loan(broker.key, SeqProxy::rawSequence(1));
        auto const sleLoan = env.le(loan);
        if (!BEAST_EXPECT(sleLoan))
            return;
        BEAST_EXPECT(sleLoan->isFlag(lsfLoanConfidential));
        BEAST_EXPECT(sleLoan->at(sfPaymentRemaining) == 12);
        BEAST_EXPECT(s.decryptLoan(loan, sfLoanAuditorEncryptedPrincipal, s.loanAuditor) == 500);
        BEAST_EXPECT(s.decryptLoan(loan, sfBrokerEncryptedPrincipal, s.brokerKey) == 500);
        BEAST_EXPECT(s.decryptLoan(loan, sfLoanAuditorEncryptedAmountOwed, s.loanAuditor) == 550);
        BEAST_EXPECT(s.decryptLoan(loan, sfBorrowerEncryptedPeriodicPayment, s.borrower) == 50);
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceSpending, s.brokerKey) == 500);
        BEAST_EXPECT(
            s.mpt.getDecryptedBalance(s.borrower, MPTTester::holderEncryptedInbox) == 500);
        BEAST_EXPECT(env.le(broker)->at(sfDebtTotal) == Number(1'000));
        BEAST_EXPECT(s.vaultField(sfAssetsAvailable) == Number(4'000));

        // A confidential Loan cannot be paid with LoanPay
        env(loan::pay(s.borrower.id(), loan.key, s.mpt(50)), Ter(tecNO_PERMISSION));

        // A regular payment below the Periodic Payment does not verify
        env(s.loanPay(loan, broker, 40, false), Ter(tecBAD_PROOF));
        env.close();

        // A regular overpayment reduces the Amount Owed by the amount paid
        auto const due = env.le(loan)->at(sfNextPaymentDueDate);
        env(s.loanPay(loan, broker, 60, false));
        env.close();
        BEAST_EXPECT(env.le(loan)->at(sfPaymentRemaining) == 11);
        BEAST_EXPECT(env.le(loan)->at(sfNextPaymentDueDate) == due + terms.paymentInterval);
        BEAST_EXPECT(env.le(loan)->at(sfConfidentialLoanVersion) == 1);
        for (auto const& [field, key] :
             {std::pair{&sfBorrowerEncryptedAmountOwed, &s.borrower},
              std::pair{&sfBrokerEncryptedAmountOwed, &s.brokerKey},
              std::pair{&sfLoanAuditorEncryptedAmountOwed, &s.loanAuditor}})
            BEAST_EXPECT(s.decryptLoan(loan, *field, *key) == 490);
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceInbox, s.brokerKey) == 60);

        // A full payment of the Amount Owed closes the Loan
        env(s.loanPay(loan, broker, 490, true));
        env.close();
        BEAST_EXPECT(env.le(loan)->at(sfPaymentRemaining) == 0);
        BEAST_EXPECT(env.le(loan)->at(~sfNextPaymentDueDate).value_or(0) == 0);
        env(s.loanPay(loan, broker, 0, true), Ter(tecKILLED));

        // Settle everything back: DebtTotal is repaid, the rest is yield
        env(loan::del(s.lender.id(), loan.key));
        env.close();
        BEAST_EXPECT(env.le(broker)->at(sfDebtTotal) == Number(1'000));
        env(s.mergeInbox(broker.key));
        env.close();
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceSpending, s.brokerKey) == 1'050);
        env(s.settle(broker, 1'051), Ter(tecBAD_PROOF));
        env(s.settle(broker, 1'050));
        env.close();
        BEAST_EXPECT(env.le(broker)->at(sfDebtTotal) == Number(0));
        BEAST_EXPECT(s.vaultField(sfAssetsAvailable) == Number(5'050));
        BEAST_EXPECT(s.vaultField(sfAssetsTotal) == Number(5'050));
        BEAST_EXPECT(s.decrypt(broker, sfConfidentialBalanceSpending, s.brokerKey) == 0);
    }

    void
    testLoanDefault()
    {
        testcase("Confidential loan default");
        using namespace jtx;

        Env env{*this, all_};
        Setup s{env};
        auto const broker = s.createBroker();
        env(s.draw(broker.key, 1'000));
        env.close();
        env(s.mergeInbox(broker.key));
        env.close();
        env(s.loanSet(broker, {.principal = 500, .owed = 550, .periodic = 50}),
            Sig(sfCounterpartySignature, s.borrower));
        env.close();
        auto const loan = keylet::loan(broker.key, SeqProxy::rawSequence(1));

        // Defaulting works as in XLS-66 and leaves the Vault and DebtTotal alone
        env(loan::manage(s.lender.id(), loan.key, tfLoanDefault), Ter(tecTOO_SOON));
        auto const due = env.le(loan)->at(sfNextPaymentDueDate) + env.le(loan)->at(sfGracePeriod);
        env.close(NetClock::time_point{NetClock::duration{due + 1}});
        env(loan::manage(s.lender.id(), loan.key, tfLoanDefault));
        env.close();
        BEAST_EXPECT(env.le(loan)->isFlag(lsfLoanDefault));
        BEAST_EXPECT(env.le(loan)->at(sfPaymentRemaining) == 0);
        BEAST_EXPECT(env.le(broker)->at(sfDebtTotal) == Number(1'000));
        BEAST_EXPECT(s.vaultField(sfAssetsTotal) == Number(5'000));
        env(s.loanPay(loan, broker, 550, true), Ter(tecKILLED));
    }

public:
    void
    run() override
    {
        testLoanLifecycle();
        testLoanDefault();
        testContextHash();
        testDisabled();
        testBrokerSet();
        testBrokerSetIssuance();
        testDraw();
        testDrawPhase();
        testMergeInbox();
    }
};

BEAST_DEFINE_TESTSUITE(ConfidentialLoanBroker, tx, xrpl);

}  // namespace xrpl::test
