#include <xrpl/tx/transactors/lending/LoanBrokerMergeInbox.h>

#include <xrpl/basics/Log.h>
#include <xrpl/beast/utility/Zero.h>
#include <xrpl/ledger/helpers/LendingHelpers.h>
#include <xrpl/ledger/helpers/TokenHelpers.h>
#include <xrpl/protocol/Asset.h>
#include <xrpl/protocol/ConfidentialTransfer.h>
#include <xrpl/protocol/Indexes.h>
#include <xrpl/protocol/LedgerFormats.h>
#include <xrpl/protocol/MPTIssue.h>
#include <xrpl/protocol/SField.h>
#include <xrpl/protocol/STLedgerEntry.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/TER.h>
#include <xrpl/protocol/XRPAmount.h>
#include <xrpl/tx/Transactor.h>

#include <utility>

namespace xrpl {

bool
LoanBrokerMergeInbox::checkExtraFeatures(PreflightContext const& ctx)
{
    return checkConfidentialLendingDependencies(ctx.rules) &&
        checkLendingProtocolDependencies(ctx.rules, ctx.tx);
}

NotTEC
LoanBrokerMergeInbox::preflight(PreflightContext const& ctx)
{
    if (ctx.tx[sfLoanBrokerID] == beast::kZero)
        return temINVALID;

    return tesSUCCESS;
}

TER
LoanBrokerMergeInbox::preclaim(PreclaimContext const& ctx)
{
    auto const sleBroker = ctx.view.read(keylet::loanBroker(ctx.tx[sfLoanBrokerID]));
    if (!sleBroker)
    {
        JLOG(ctx.j.warn()) << "LoanBroker does not exist.";
        return tecNO_ENTRY;
    }
    if (ctx.tx[sfAccount] != sleBroker->at(sfOwner))
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
        JLOG(ctx.j.fatal()) << "Vault is missing for Broker " << ctx.tx[sfLoanBrokerID];
        return tefBAD_LEDGER;
        // LCOV_EXCL_STOP
    }

    Asset const vaultAsset = sleVault->at(sfAsset);
    auto const brokerPseudo = sleBroker->at(sfAccount);

    if (auto const ter = checkFrozen(ctx.view, brokerPseudo, vaultAsset))
        return ter;

    if (auto const ter = requireAuth(ctx.view, vaultAsset, brokerPseudo))
        return ter;

    return tesSUCCESS;
}

TER
LoanBrokerMergeInbox::doApply()
{
    auto const broker = view().read(keylet::loanBroker(ctx_.tx[sfLoanBrokerID]));
    if (!broker)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE
    auto const vault = view().read(keylet::vault(broker->at(sfVaultID)));
    if (!vault)
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE

    Asset const vaultAsset = vault->at(sfAsset);
    auto const issuanceID = vaultAsset.get<MPTIssue>().getMptID();
    auto const brokerPseudo = broker->at(sfAccount);

    auto const lending = view().peek(keylet::mptoken(issuanceID, brokerPseudo));
    if (!lending || !lending->isFieldPresent(sfConfidentialBalanceSpending) ||
        !lending->isFieldPresent(sfConfidentialBalanceInbox) ||
        !lending->isFieldPresent(sfHolderEncryptionKey))
        return tefBAD_LEDGER;  // LCOV_EXCL_LINE

    // spending = spending + inbox
    auto sum = homomorphicAdd(
        (*lending)[sfConfidentialBalanceSpending], (*lending)[sfConfidentialBalanceInbox]);
    if (!sum)
    {
        // LCOV_EXCL_START
        JLOG(j_.error()) << "LoanBrokerMergeInbox failed homomorphic add for inbox merge.";
        return tecINTERNAL;
        // LCOV_EXCL_STOP
    }
    (*lending)[sfConfidentialBalanceSpending] = std::move(*sum);

    // Reset the inbox to the canonical encrypted zero for the pseudo-account
    auto zero =
        encryptCanonicalZeroAmount((*lending)[sfHolderEncryptionKey], brokerPseudo, issuanceID);
    if (!zero)
        return tecINTERNAL;  // LCOV_EXCL_LINE
    (*lending)[sfConfidentialBalanceInbox] = std::move(*zero);

    incrementConfidentialVersion(*lending);

    view().update(lending);
    return tesSUCCESS;
}

void
LoanBrokerMergeInbox::visitInvariantEntry(bool, SLE::ConstRef, SLE::ConstRef)
{
    // Checked by ValidConfidentialMPToken, as for ConfidentialMPTMergeInbox.
}

bool
LoanBrokerMergeInbox::finalizeInvariants(
    STTx const&,
    TER,
    XRPAmount,
    ReadView const&,
    beast::Journal const&)
{
    return true;
}

}  // namespace xrpl
