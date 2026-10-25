#include <xrpl/tx/transactors/lending/LoanBrokerDraw.h>

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

#include <cstdint>
#include <optional>
#include <utility>

namespace xrpl {

bool
LoanBrokerDraw::checkExtraFeatures(PreflightContext const& ctx)
{
    return checkConfidentialLendingDependencies(ctx.rules) &&
        checkLendingProtocolDependencies(ctx.rules, ctx.tx);
}

NotTEC
LoanBrokerDraw::preflight(PreflightContext const& ctx)
{
    auto const amount = ctx.tx[sfAmount];
    if (!amount.holds<MPTIssue>() || amount <= beast::kZero)
        return temBAD_AMOUNT;

    if (ctx.tx[sfLoanBrokerID] == beast::kZero)
        return temINVALID;

    return checkEncryptedAmountFormat(ctx.tx);
}

TER
LoanBrokerDraw::preclaim(PreclaimContext const& ctx)
{
    auto const& tx = ctx.tx;
    auto const amount = tx[sfAmount];

    auto const sleBroker = ctx.view.read(keylet::loanBroker(tx[sfLoanBrokerID]));
    if (!sleBroker)
    {
        JLOG(ctx.j.warn()) << "LoanBroker does not exist.";
        return tecNO_ENTRY;
    }
    if (tx[sfAccount] != sleBroker->at(sfOwner))
    {
        JLOG(ctx.j.warn()) << "Account is not the owner of the LoanBroker.";
        return tecNO_PERMISSION;
    }
    if (!sleBroker->isFlag(lsfLoanBrokerConfidential))
    {
        JLOG(ctx.j.warn()) << "LoanBroker is not confidential.";
        return tecNO_PERMISSION;
    }

    auto const sleVault = ctx.view.read(keylet::vault(sleBroker->at(sfVaultID)));
    if (!sleVault)
    {
        // LCOV_EXCL_START
        JLOG(ctx.j.fatal()) << "Vault is missing for Broker " << tx[sfLoanBrokerID];
        return tefBAD_LEDGER;
        // LCOV_EXCL_STOP
    }

    Asset const vaultAsset = sleVault->at(sfAsset);
    if (amount.asset() != vaultAsset)
        return tecWRONG_ASSET;

    if (getVaultPhase(ctx.view, sleVault) != VaultPhase::Investment)
    {
        JLOG(ctx.j.warn()) << "Vault is not in its Investment phase.";
        return tecNO_PERMISSION;
    }

    Number const drawn = amount;
    if (sleVault->at(sfAssetsAvailable) < drawn)
    {
        JLOG(ctx.j.warn()) << "Insufficient assets available in the Vault.";
        return tecINSUFFICIENT_FUNDS;
    }

    if (Number const debtMaximum = sleBroker->at(sfDebtMaximum);
        debtMaximum != 0 && sleBroker->at(sfDebtTotal) + drawn > debtMaximum)
    {
        JLOG(ctx.j.warn()) << "Draw would exceed the LoanBroker's DebtMaximum.";
        return tecLIMIT_EXCEEDED;
    }

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
    if (!sleIssuance || !sleLending || !sleLending->isFieldPresent(sfHolderEncryptionKey) ||
        !sleIssuance->isFieldPresent(sfIssuerEncryptionKey) ||
        !sleIssuance->isFieldPresent(sfAuditorEncryptionKey))
    {
        // LCOV_EXCL_START
        JLOG(ctx.j.fatal()) << "Lending Balance is missing for Broker " << tx[sfLoanBrokerID];
        return tefBAD_LEDGER;
        // LCOV_EXCL_STOP
    }

    auto const blindingFactor = tx[sfBlindingFactor];
    if (!isTesSuccess(verifyRevealedAmount(
            static_cast<std::uint64_t>(amount.mpt().value()),
            Slice(blindingFactor.data(), blindingFactor.size()),
            {
                .publicKey = (*sleLending)[sfHolderEncryptionKey],
                .encryptedAmount = tx[sfHolderEncryptedAmount],
            },
            {
                .publicKey = (*sleIssuance)[sfIssuerEncryptionKey],
                .encryptedAmount = tx[sfIssuerEncryptedAmount],
            },
            ConfidentialRecipient{
                .publicKey = (*sleIssuance)[sfAuditorEncryptionKey],
                .encryptedAmount = tx[sfAuditorEncryptedAmount],
            })))
    {
        JLOG(ctx.j.warn()) << "Ciphertexts do not encrypt the drawn amount.";
        return tecBAD_PROOF;
    }

    return tesSUCCESS;
}

TER
LoanBrokerDraw::doApply()
{
    auto const& tx = ctx_.tx;
    auto& view = ctx_.view();

    auto const amount = tx[sfAmount];
    auto const drawn = static_cast<std::uint64_t>(amount.mpt().value());
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

    // Take the amount from the Vault pseudo-account's public balance
    auto const vaultBalance = (*vaultHolding)[sfMPTAmount];
    if (vaultBalance < drawn)
        return tecINTERNAL;  // LCOV_EXCL_LINE
    (*vaultHolding)[sfMPTAmount] = vaultBalance - drawn;

    // and move it into confidential circulation
    auto const coa = (*issuance)[~sfConfidentialOutstandingAmount].valueOr(0);
    if (drawn > kMaxMpTokenAmount - coa)
        return tecINTERNAL;  // LCOV_EXCL_LINE
    (*issuance)[sfConfidentialOutstandingAmount] = coa + drawn;

    // Credit the Lending Balance's inbox and its issuer and auditor mirrors
    for (auto const& [field, ciphertext] :
         {std::pair{&sfConfidentialBalanceInbox, tx[sfHolderEncryptedAmount]},
          std::pair{&sfIssuerEncryptedBalance, tx[sfIssuerEncryptedAmount]},
          std::pair{&sfAuditorEncryptedBalance, tx[sfAuditorEncryptedAmount]}})
    {
        auto sum = homomorphicAdd(ciphertext, (*lending)[*field]);
        if (!sum)
        {
            // LCOV_EXCL_START
            JLOG(j_.error()) << "LoanBrokerDraw failed homomorphic add for " << field->getName();
            return tecINTERNAL;
            // LCOV_EXCL_STOP
        }
        (*lending)[*field] = std::move(*sum);
    }

    // The Lending Balance now owes the Vault the drawn amount
    auto const vaultScale = getVaultBaseScale(vault);
    vault->at(sfAssetsAvailable) -= Number(amount);
    adjustBrokerDebtTotal(broker, vault, Number(amount), vaultScale);

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
LoanBrokerDraw::visitInvariantEntry(bool isDelete, SLE::ConstRef before, SLE::ConstRef after)
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
LoanBrokerDraw::finalizeInvariants(
    STTx const& tx,
    TER result,
    XRPAmount,
    ReadView const&,
    beast::Journal const& j)
{
    if (!isTesSuccess(result))
        return true;

    auto const amount = tx[sfAmount];
    auto const drawn = static_cast<std::uint64_t>(amount.mpt().value());

    auto const delta = [](auto const& change) {
        return change.before && change.after ? std::optional{*change.after - *change.before}
                                             : std::nullopt;
    };

    if (delta(assetsAvailable_) != -Number(amount) || delta(assetsTotal_) != Number(0))
    {
        JLOG(j.fatal()) << "Invariant failed: LoanBrokerDraw must decrease Vault AssetsAvailable "
                           "by the drawn amount and leave AssetsTotal unchanged";
        return false;
    }

    if (delta(debtTotal_) != Number(amount))
    {
        JLOG(j.fatal()) << "Invariant failed: LoanBrokerDraw must increase DebtTotal by the "
                           "drawn amount";
        return false;
    }

    if (!confidentialOutstanding_.before || !confidentialOutstanding_.after ||
        *confidentialOutstanding_.after - *confidentialOutstanding_.before != drawn ||
        outstanding_.before != outstanding_.after)
    {
        JLOG(j.fatal()) << "Invariant failed: LoanBrokerDraw must increase "
                           "ConfidentialOutstandingAmount by the drawn amount and leave "
                           "OutstandingAmount unchanged";
        return false;
    }

    auto const vaultHolding =
        vaultPseudo_ ? mptAmounts_.find(*vaultPseudo_) : mptAmounts_.end();
    if (vaultHolding == mptAmounts_.end() || !vaultHolding->second.before ||
        !vaultHolding->second.after ||
        *vaultHolding->second.before - *vaultHolding->second.after != drawn)
    {
        JLOG(j.fatal()) << "Invariant failed: LoanBrokerDraw must decrease the Vault "
                           "pseudo-account's MPTAmount by the drawn amount";
        return false;
    }

    return true;
}

}  // namespace xrpl
