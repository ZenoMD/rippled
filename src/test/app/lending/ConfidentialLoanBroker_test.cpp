#include <test/jtx/Account.h>
#include <test/jtx/Env.h>
#include <test/jtx/TestHelpers.h>
#include <test/jtx/amount.h>
#include <test/jtx/mpt.h>
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

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace xrpl::test {

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
        jtx::MPTTester mpt;
        Keylet vaultKeylet = keylet::vault(beast::kZero);

        Setup(
            jtx::Env& env_,
            bool investment = true,
            std::uint32_t mptFlags = tfMPTCanTransfer | tfMPTCanLock |
                tfMPTCanHoldConfidentialBalance,
            bool withAuditor = true)
            : env(env_), mpt(env, issuer, {.holders = {lender}, .auditor = auditor})
        {
            using namespace jtx;

            mpt.create({.ownerCount = 1, .flags = mptFlags});
            mpt.authorize({.account = lender});
            mpt.pay(issuer, lender, 10'000);

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
        BEAST_EXPECT(s.mpt.getIssuanceConfidentialBalance() == 1'000);
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
        BEAST_EXPECT(s.mpt.getIssuanceConfidentialBalance() == 1'000);
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

public:
    void
    run() override
    {
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
