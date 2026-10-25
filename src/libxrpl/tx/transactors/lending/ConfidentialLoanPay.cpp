#include <xrpl/tx/transactors/lending/ConfidentialLoanPay.h>

#include <xrpl/basics/Buffer.h>
#include <xrpl/basics/Log.h>
#include <xrpl/basics/Slice.h>
#include <xrpl/beast/utility/Zero.h>
#include <xrpl/ledger/helpers/LendingHelpers.h>
#include <xrpl/ledger/helpers/TokenHelpers.h>
#include <xrpl/protocol/Asset.h>
#include <xrpl/protocol/ConfidentialTransfer.h>
#include <xrpl/protocol/Indexes.h>
#include <xrpl/protocol/LedgerFormats.h>
#include <xrpl/protocol/MPTIssue.h>
#include <xrpl/protocol/Protocol.h>
#include <xrpl/protocol/SField.h>
#include <xrpl/protocol/STLedgerEntry.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/TER.h>
#include <xrpl/protocol/TxFlags.h>
#include <xrpl/protocol/XRPAmount.h>
#include <xrpl/tx/Transactor.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace xrpl {

namespace {

// The payment's ciphertexts, which share one randomness
constexpr auto kPaymentCiphertexts = std::to_array<SF_VL const*>({
    &sfSenderEncryptedAmount,
    &sfDestinationEncryptedAmount,
    &sfIssuerEncryptedAmount,
    &sfAuditorEncryptedAmount,
    &sfLoanAuditorEncryptedAmount,
});

bool
isFullPayment(STTx const& tx)
{
    return tx.isFlag(tfLoanFullPayment);
}

}  // namespace

bool
ConfidentialLoanPay::checkExtraFeatures(PreflightContext const& ctx)
{
    return checkConfidentialLendingDependencies(ctx.rules) &&
        checkLendingProtocolDependencies(ctx.rules, ctx.tx);
}

std::uint32_t
ConfidentialLoanPay::getFlagsMask(PreflightContext const&)
{
    return tfConfidentialLoanPayMask;
}

XRPAmount
ConfidentialLoanPay::calculateBaseFee(ReadView const& view, STTx const& tx)
{
    return Transactor::calculateBaseFee(view, tx, kConfidentialFeeMultiplier);
}

NotTEC
ConfidentialLoanPay::preflight(PreflightContext const& ctx)
{
    auto const& tx = ctx.tx;
    if (tx[sfLoanID] == beast::kZero)
        return temINVALID;

    for (auto const* field : kPaymentCiphertexts)
    {
        if (tx[*field].size() != kEcGamalEncryptedTotalLength || !isValidCiphertext(tx[*field]))
            return temBAD_CIPHERTEXT;
    }

    bool const full = isFullPayment(tx);
    for (auto const* field : {&sfAmountOwedCommitment, &sfPeriodicPaymentCommitment})
    {
        // Required for a regular payment, not allowed for a full payment
        if (tx.isFieldPresent(*field) == full)
            return temMALFORMED;
        if (!full && !isValidCompressedECPoint(tx[*field]))
            return temMALFORMED;
    }
    if (!isValidCompressedECPoint(tx[sfBalanceCommitment]) ||
        !isValidCompressedECPoint(tx[sfAmountCommitment]))
        return temMALFORMED;
    if (tx[sfZKProof].size() != (full ? kFullProofLength : kRegularProofLength))
        return temMALFORMED;

    // The payment's ciphertexts share their first 33 bytes (spec §13.5 #4)
    Slice const c1{tx[sfSenderEncryptedAmount].data(), kEcCiphertextComponentLength};
    for (auto const* field : kPaymentCiphertexts)
    {
        if (Slice{tx[*field].data(), kEcCiphertextComponentLength} != c1)
            return temMALFORMED;
    }

    return tesSUCCESS;
}

TER
ConfidentialLoanPay::preclaim(PreclaimContext const& ctx)
{
    auto const& tx = ctx.tx;
    bool const full = isFullPayment(tx);

    auto const loan = ctx.view.read(keylet::loan(tx[sfLoanID]));
    if (!loan)
    {
        JLOG(ctx.j.warn()) << "Loan does not exist.";
        return tecNO_ENTRY;
    }
    if (loan->at(sfBorrower) != tx[sfAccount] || !loan->isFlag(lsfLoanConfidential))
    {
        JLOG(ctx.j.warn()) << "Account is not the Borrower of a confidential Loan.";
        return tecNO_PERMISSION;
    }
    if (auto const remaining = loan->at(sfPaymentRemaining); remaining == 0 || (remaining == 1 && !full))
    {
        JLOG(ctx.j.warn()) << "Loan is closed, or its final payment must be a full payment.";
        return tecKILLED;
    }

    auto const broker = ctx.view.read(keylet::loanBroker(loan->at(sfLoanBrokerID)));
    if (!broker)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE
    auto const vault = ctx.view.read(keylet::vault(broker->at(sfVaultID)));
    if (!vault)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE
    Asset const asset = vault->at(sfAsset);
    auto const issuanceID = asset.get<MPTIssue>().getMptID();
    auto const brokerPseudo = broker->at(sfAccount);
    auto const borrower = tx[sfAccount];

    for (auto const& account : {borrower, brokerPseudo})
    {
        if (auto const ter = checkFrozen(ctx.view, account, asset))
            return ter;
    }
    for (auto const& account : {borrower, brokerPseudo})
    {
        if (auto const ter = requireAuth(ctx.view, asset, account))
            return ter;
    }

    auto const issuance = ctx.view.read(keylet::mptokenIssuance(issuanceID));
    auto const lending = ctx.view.read(keylet::mptoken(issuanceID, brokerPseudo));
    auto const borrowerToken = ctx.view.read(keylet::mptoken(issuanceID, borrower));
    if (!issuance || !lending || !borrowerToken ||
        !borrowerToken->isFieldPresent(sfConfidentialBalanceSpending))
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE

    auto const contextHash = getConfidentialLoanPayContextHash(
        borrower,
        issuanceID,
        tx.getSeqProxy().value(),
        brokerPseudo,
        (*borrowerToken)[~sfConfidentialBalanceVersion].value_or(0),
        loan->key(),
        loan->at(~sfConfidentialLoanVersion).value_or(0));

    Slice const proof = tx[sfZKProof];
    auto const part = [&proof](std::size_t offset, std::size_t size) {
        return Slice{proof.data() + offset, size};
    };
    Slice const borrowerKey = (*borrowerToken)[sfHolderEncryptionKey];
    Slice const spending = (*borrowerToken)[sfConfidentialBalanceSpending];
    Slice const amountCommitment = tx[sfAmountCommitment];
    Slice const balanceCommitment = tx[sfBalanceCommitment];

    // Run every check before returning, as XLS-96 does.
    bool valid = true;
    auto const check = [&valid](TER ter) {
        if (!isTesSuccess(ter))
            valid = false;
    };

    // Part 1: the payment transfer, as an XLS-96 Send
    check(verifySendProof(
        part(0, kEcSendProofLength),
        {.publicKey = borrowerKey, .encryptedAmount = tx[sfSenderEncryptedAmount]},
        {.publicKey = (*lending)[sfHolderEncryptionKey],
         .encryptedAmount = tx[sfDestinationEncryptedAmount]},
        {.publicKey = (*issuance)[sfIssuerEncryptionKey],
         .encryptedAmount = tx[sfIssuerEncryptedAmount]},
        ConfidentialRecipient{
            .publicKey = (*issuance)[sfAuditorEncryptionKey],
            .encryptedAmount = tx[sfAuditorEncryptedAmount]},
        spending,
        amountCommitment,
        balanceCommitment,
        contextHash));

    // Part 2: the Loan Auditor's copy of the payment
    std::size_t offset = kEcSendProofLength;
    check(verifyCompactSigmaProof(
        part(offset, kEcSendSigmaProofLength),
        {{.publicKey = loan->at(sfLoanAuditorEncryptionKey),
          .encryptedAmount = tx[sfLoanAuditorEncryptedAmount]}},
        amountCommitment,
        borrowerKey,
        spending,
        balanceCommitment,
        contextHash));
    offset += kEcSendSigmaProofLength;

    if (full)
    {
        // Part 3: the payment equals the Amount Owed
        auto const difference = homomorphicSubtract(
            tx[sfSenderEncryptedAmount], loan->at(sfBorrowerEncryptedAmountOwed));
        if (!difference)
            return tecBAD_PROOF;
        check(verifyClawbackProof(
            0, part(offset, kEcClawbackProofLength), borrowerKey, *difference, contextHash));
    }
    else
    {
        // Parts 3-4: link fresh commitments to the Amount Owed and Periodic Payment
        Slice const owed = tx[sfAmountOwedCommitment];
        Slice const periodic = tx[sfPeriodicPaymentCommitment];
        check(verifyCommitmentLinkProof(
            part(offset, kEcConvertBackSigmaProofLength),
            borrowerKey,
            loan->at(sfBorrowerEncryptedAmountOwed),
            owed,
            contextHash));
        offset += kEcConvertBackSigmaProofLength;
        check(verifyCommitmentLinkProof(
            part(offset, kEcConvertBackSigmaProofLength),
            borrowerKey,
            loan->at(sfBorrowerEncryptedPeriodicPayment),
            periodic,
            contextHash));
        offset += kEcConvertBackSigmaProofLength;

        // Part 5: Periodic Payment <= X <= Amount Owed
        auto const overMinimum = combinePoints(amountCommitment, 1, periodic, -1);
        auto const underOwed = combinePoints(owed, 1, amountCommitment, -1);
        if (!overMinimum || !underOwed)
            return tecBAD_PROOF;
        check(verifyRangeProof(
            part(offset, kEcDoubleBulletproofLength), {*overMinimum, *underOwed}, contextHash));
    }

    return valid ? TER{tesSUCCESS} : TER{tecBAD_PROOF};
}

TER
ConfidentialLoanPay::doApply()
{
    auto const& tx = ctx_.tx;
    auto& view = ctx_.view();

    auto const loan = view.peek(keylet::loan(tx[sfLoanID]));
    if (!loan)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE
    auto const broker = view.read(keylet::loanBroker(loan->at(sfLoanBrokerID)));
    if (!broker)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE
    auto const vault = view.read(keylet::vault(broker->at(sfVaultID)));
    if (!vault)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE
    Asset const asset = vault->at(sfAsset);
    auto const issuanceID = asset.get<MPTIssue>().getMptID();

    auto const issuance = view.read(keylet::mptokenIssuance(issuanceID));
    auto const borrowerToken = view.peek(keylet::mptoken(issuanceID, accountID_));
    auto const lending = view.peek(keylet::mptoken(issuanceID, broker->at(sfAccount)));
    if (!issuance || !borrowerToken || !lending)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE

    // 1. Move the payment from the Borrower to the Lending Balance's inbox
    Slice const proof = tx[sfZKProof];
    if (auto const ter = moveConfidentialAmount(
            view,
            issuance,
            borrowerToken,
            lending,
            tx[sfSenderEncryptedAmount],
            tx[sfDestinationEncryptedAmount],
            tx[sfIssuerEncryptedAmount],
            tx[sfAuditorEncryptedAmount],
            Slice{proof.data(), kEcBlindingFactorLength}))
        return ter;

    if (isFullPayment(tx))
    {
        // 3. A full payment closes the Loan
        loan->at(sfPaymentRemaining) = 0;
        loan->at(sfNextPaymentDueDate) = 0;
    }
    else
    {
        // 2. Reduce the Amount Owed by the payment under each key
        for (auto const& [owedField, paymentField] :
             {std::pair{&sfBorrowerEncryptedAmountOwed, &sfSenderEncryptedAmount},
              std::pair{&sfBrokerEncryptedAmountOwed, &sfDestinationEncryptedAmount},
              std::pair{&sfLoanAuditorEncryptedAmountOwed, &sfLoanAuditorEncryptedAmount}})
        {
            auto const current = loan->getFieldVL(*owedField);
            auto owed = homomorphicSubtract(makeSlice(current), tx[*paymentField]);
            if (!owed)
                return tecBAD_PROOF;
            loan->setFieldVL(*owedField, *owed);
        }
        loan->setFieldU32(
            sfConfidentialLoanVersion, loan->getFieldU32(sfConfidentialLoanVersion) + 1);

        loan->at(sfPaymentRemaining) = loan->at(sfPaymentRemaining) - 1;
        auto const nextDue = loan->at(sfNextPaymentDueDate);
        loan->at(sfPreviousPaymentDueDate) = *nextDue;
        loan->at(sfNextPaymentDueDate) = *nextDue + loan->at(sfPaymentInterval);
    }

    view.update(loan);
    return tesSUCCESS;
}

void
ConfidentialLoanPay::visitInvariantEntry(bool, SLE::ConstRef before, SLE::ConstRef after)
{
    auto const& sle = after ? after : before;
    if (!sle)
        return;
    if (sle->getType() == ltVAULT)
        vaultModified_ = true;
    if (sle->getType() == ltLOAN_BROKER && before && after &&
        before->at(sfDebtTotal) != after->at(sfDebtTotal))
        brokerDebtChanged_ = true;
}

bool
ConfidentialLoanPay::finalizeInvariants(
    STTx const&,
    TER result,
    XRPAmount,
    ReadView const&,
    beast::Journal const& j)
{
    // Supply is checked by ValidConfidentialMPToken (spec §13.7)
    if (isTesSuccess(result) && (vaultModified_ || brokerDebtChanged_))
    {
        JLOG(j.fatal())
            << "Invariant failed: ConfidentialLoanPay must not change the Vault or DebtTotal";
        return false;
    }
    return true;
}

}  // namespace xrpl
