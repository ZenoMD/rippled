#include <xrpl/tx/transactors/lending/LoanBrokerSettle.h>

#include <xrpl/basics/Log.h>
#include <xrpl/basics/Number.h>
#include <xrpl/basics/Slice.h>
#include <xrpl/beast/utility/Zero.h>
#include <xrpl/ledger/helpers/LendingHelpers.h>
#include <xrpl/ledger/helpers/TokenHelpers.h>
#include <xrpl/ledger/helpers/VaultHelpers.h>
#include <xrpl/protocol/Asset.h>
#include <xrpl/protocol/ConfidentialTransfer.h>
#include <xrpl/protocol/Indexes.h>
#include <xrpl/protocol/LedgerFormats.h>
#include <xrpl/protocol/MPTIssue.h>
#include <xrpl/protocol/Protocol.h>
#include <xrpl/protocol/SField.h>
#include <xrpl/protocol/STAmount.h>
#include <xrpl/protocol/STLedgerEntry.h>
#include <xrpl/protocol/STTakesAsset.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/TER.h>
#include <xrpl/protocol/XRPAmount.h>
#include <xrpl/tx/Transactor.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <utility>

namespace xrpl {

bool
LoanBrokerSettle::checkExtraFeatures(PreflightContext const& ctx)
{
    return checkConfidentialLendingDependencies(ctx.rules) &&
        checkLendingProtocolDependencies(ctx.rules, ctx.tx);
}

NotTEC
LoanBrokerSettle::preflight(PreflightContext const& ctx)
{
    auto const amount = ctx.tx[sfAmount];
    if (!amount.holds<MPTIssue>() || amount <= beast::kZero)
        return temBAD_AMOUNT;

    if (ctx.tx[sfLoanBrokerID] == beast::kZero)
        return temINVALID;

    if (auto const ret = checkEncryptedAmountFormat(ctx.tx))
        return ret;

    if (!isValidCompressedECPoint(ctx.tx[sfBalanceCommitment]) ||
        ctx.tx[sfZKProof].size() != kEcConvertBackProofLength)
        return temMALFORMED;

    return tesSUCCESS;
}

TER
LoanBrokerSettle::preclaim(PreclaimContext const& ctx)
{
    auto const& tx = ctx.tx;
    auto const amount = tx[sfAmount];

    auto const sleBroker = ctx.view.read(keylet::loanBroker(tx[sfLoanBrokerID]));
    if (!sleBroker)
    {
        JLOG(ctx.j.warn()) << "LoanBroker does not exist.";
        return tecNO_ENTRY;
    }
    if (tx[sfAccount] != sleBroker->at(sfOwner) || !sleBroker->isFlag(lsfLoanBrokerConfidential))
    {
        JLOG(ctx.j.warn()) << "Account is not the owner of a confidential LoanBroker.";
        return tecNO_PERMISSION;
    }

    auto const sleVault = ctx.view.read(keylet::vault(sleBroker->at(sfVaultID)));
    if (!sleVault)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE

    Asset const vaultAsset = sleVault->at(sfAsset);
    if (amount.asset() != vaultAsset)
        return tecWRONG_ASSET;

    auto const vaultPseudo = sleVault->at(sfAccount);
    auto const brokerPseudo = sleBroker->at(sfAccount);
    for (auto const& account : {vaultPseudo, brokerPseudo})
    {
        if (auto const ter = checkFrozen(ctx.view, account, vaultAsset))
            return ter;
    }
    for (auto const& account : {vaultPseudo, brokerPseudo})
    {
        if (auto const ter = requireAuth(ctx.view, vaultAsset, account))
            return ter;
    }

    auto const issuanceID = amount.get<MPTIssue>().getMptID();
    auto const sleIssuance = ctx.view.read(keylet::mptokenIssuance(issuanceID));
    auto const sleLending = ctx.view.read(keylet::mptoken(issuanceID, brokerPseudo));
    if (!sleIssuance || !sleLending)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE

    Slice const brokerKey = (*sleLending)[sfHolderEncryptionKey];
    auto const settled = static_cast<std::uint64_t>(amount.mpt().value());
    auto const blindingFactor = tx[sfBlindingFactor];

    // Run both checks before returning, as XLS-96 does.
    bool const revealed = isTesSuccess(verifyRevealedAmount(
        settled,
        Slice(blindingFactor.data(), blindingFactor.size()),
        {.publicKey = brokerKey, .encryptedAmount = tx[sfHolderEncryptedAmount]},
        {.publicKey = (*sleIssuance)[sfIssuerEncryptionKey],
         .encryptedAmount = tx[sfIssuerEncryptedAmount]},
        ConfidentialRecipient{
            .publicKey = (*sleIssuance)[sfAuditorEncryptionKey],
            .encryptedAmount = tx[sfAuditorEncryptedAmount]}));

    auto const contextHash = getLoanBrokerSettleContextHash(
        brokerPseudo,
        issuanceID,
        tx.getSeqProxy().value(),
        (*sleLending)[~sfConfidentialBalanceVersion].value_or(0));
    bool const proven = isTesSuccess(verifyConvertBackProof(
        tx[sfZKProof],
        brokerKey,
        (*sleLending)[sfConfidentialBalanceSpending],
        tx[sfBalanceCommitment],
        settled,
        contextHash));

    if (!revealed || !proven)
    {
        JLOG(ctx.j.warn()) << "Settlement proof does not verify.";
        return tecBAD_PROOF;
    }

    return tesSUCCESS;
}

TER
LoanBrokerSettle::doApply()
{
    auto const& tx = ctx_.tx;
    auto& view = ctx_.view();

    auto const amount = tx[sfAmount];
    auto const settled = static_cast<std::uint64_t>(amount.mpt().value());
    auto const issuanceID = amount.get<MPTIssue>().getMptID();

    auto const broker = view.peek(keylet::loanBroker(tx[sfLoanBrokerID]));
    if (!broker)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE
    auto const vault = view.peek(keylet::vault(broker->at(sfVaultID)));
    auto const issuance = view.peek(keylet::mptokenIssuance(issuanceID));
    if (!vault || !issuance)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE
    auto const vaultHolding = view.peek(keylet::mptoken(issuanceID, vault->at(sfAccount)));
    auto const lending = view.peek(keylet::mptoken(issuanceID, broker->at(sfAccount)));
    if (!vaultHolding || !lending)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE

    // 1. Debit the Lending Balance's spending balance and mirrors
    for (auto const& [field, ciphertext] :
         {std::pair{&sfConfidentialBalanceSpending, tx[sfHolderEncryptedAmount]},
          std::pair{&sfIssuerEncryptedBalance, tx[sfIssuerEncryptedAmount]},
          std::pair{&sfAuditorEncryptedBalance, tx[sfAuditorEncryptedAmount]}})
    {
        auto result = homomorphicSubtract((*lending)[*field], ciphertext);
        if (!result)
            return tecINTERNAL;  // LCOV_EXCL_LINE
        (*lending)[*field] = std::move(*result);
    }
    incrementConfidentialVersion(*lending);

    // 2. Take the amount out of confidential circulation
    auto const coa = (*issuance)[~sfConfidentialOutstandingAmount].valueOr(0);
    if (settled > coa)
        return tecINTERNAL;  // LCOV_EXCL_LINE
    (*issuance)[sfConfidentialOutstandingAmount] = coa - settled;

    // 3. Credit the Vault pseudo-account's public balance
    (*vaultHolding)[sfMPTAmount] = (*vaultHolding)[sfMPTAmount] + settled;
    vault->at(sfAssetsAvailable) += Number(amount);

    // 4. Repay DebtTotal first; the rest is yield to the Vault
    Number const debt = broker->at(sfDebtTotal);
    Number const repaid = std::min(Number(amount), debt);
    auto const vaultScale = getVaultBaseScale(vault);
    adjustBrokerDebtTotal(broker, vault, -repaid, vaultScale);
    vault->at(sfAssetsTotal) += Number(amount) - repaid;

    Asset const vaultAsset = vault->at(sfAsset);
    associateAsset(*vault, vaultAsset);
    associateAsset(*broker, vaultAsset);

    view.update(vaultHolding);
    view.update(lending);
    view.update(issuance);
    view.update(vault);
    view.update(broker);

    return tesSUCCESS;
}

void
LoanBrokerSettle::visitInvariantEntry(bool isDelete, SLE::ConstRef before, SLE::ConstRef after)
{
    auto const record = [&](SLE::ConstRef sle, bool isAfter) {
        if (!sle)
            return;
        auto const set = [isAfter](auto& change, auto value) {
            (isAfter ? change.after : change.before) = value;
        };
        switch (sle->getType())
        {
            case ltVAULT:
                set(assetsAvailable_, Number(sle->at(sfAssetsAvailable)));
                set(assetsTotal_, Number(sle->at(sfAssetsTotal)));
                vaultPseudo_ = sle->at(sfAccount);
                break;
            case ltLOAN_BROKER:
                set(debtTotal_, Number(sle->at(sfDebtTotal)));
                break;
            case ltMPTOKEN_ISSUANCE:
                set(confidentialOutstanding_,
                    (*sle)[~sfConfidentialOutstandingAmount].value_or(0));
                set(outstanding_, sle->getFieldU64(sfOutstandingAmount));
                break;
            case ltMPTOKEN:
                set(mptAmounts_[sle->at(sfAccount)], sle->getFieldU64(sfMPTAmount));
                break;
            default:
                break;
        }
    };

    record(before, false);
    if (!isDelete)
        record(after, true);
}

bool
LoanBrokerSettle::finalizeInvariants(
    STTx const& tx,
    TER result,
    XRPAmount,
    ReadView const&,
    beast::Journal const& j)
{
    if (!isTesSuccess(result))
        return true;

    // Spec §8.5
    Number const amount = tx[sfAmount];
    auto const settled = static_cast<std::uint64_t>(tx[sfAmount].mpt().value());
    auto const delta = [](auto const& change) {
        return change.before && change.after ? std::optional{*change.after - *change.before}
                                             : std::nullopt;
    };

    auto const vaultHolding =
        vaultPseudo_ ? mptAmounts_.find(*vaultPseudo_) : mptAmounts_.end();
    bool const good = delta(assetsAvailable_) == amount && delta(debtTotal_) &&
        delta(assetsTotal_) && *delta(assetsTotal_) - *delta(debtTotal_) == amount &&
        confidentialOutstanding_.before && confidentialOutstanding_.after &&
        *confidentialOutstanding_.before - *confidentialOutstanding_.after == settled &&
        outstanding_.before == outstanding_.after && vaultHolding != mptAmounts_.end() &&
        vaultHolding->second.before && vaultHolding->second.after &&
        *vaultHolding->second.after - *vaultHolding->second.before == settled;
    if (!good)
    {
        JLOG(j.fatal()) << "Invariant failed: LoanBrokerSettle must move exactly Amount from the "
                           "Lending Balance to the Vault, repaying DebtTotal first";
        return false;
    }
    return true;
}

}  // namespace xrpl
