#include <xrpl/tx/transactors/lending/LoanBrokerSet.h>

#include <xrpl/basics/Log.h>
#include <xrpl/basics/Number.h>
#include <xrpl/beast/utility/Zero.h>
#include <xrpl/core/ServiceRegistry.h>
#include <xrpl/ledger/View.h>
#include <xrpl/ledger/helpers/AccountRootHelpers.h>
#include <xrpl/ledger/helpers/LendingHelpers.h>
#include <xrpl/ledger/helpers/TokenHelpers.h>
#include <xrpl/ledger/helpers/VaultHelpers.h>
#include <xrpl/protocol/AccountID.h>
#include <xrpl/protocol/Asset.h>
#include <xrpl/protocol/ConfidentialTransfer.h>
#include <xrpl/protocol/Feature.h>
#include <xrpl/protocol/Indexes.h>
#include <xrpl/protocol/LedgerFormats.h>
#include <xrpl/protocol/Protocol.h>
#include <xrpl/protocol/SField.h>
#include <xrpl/protocol/STAmount.h>
#include <xrpl/protocol/STLedgerEntry.h>
#include <xrpl/protocol/STNumber.h>
#include <xrpl/protocol/STTakesAsset.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/TER.h>
#include <xrpl/protocol/TxFlags.h>
#include <xrpl/protocol/XRPAmount.h>
#include <xrpl/tx/Transactor.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace xrpl {

bool
LoanBrokerSet::checkExtraFeatures(PreflightContext const& ctx)
{
    if (!ctx.rules.enabled(featureLendingProtocolV1_2) && ctx.tx.isFieldPresent(sfDomainID))
        return false;
    // tfLoanBrokerConfidential is gated by getFlagsMask
    bool const confidentialFields =
        ctx.tx.isFieldPresent(sfHolderEncryptionKey) || ctx.tx.isFieldPresent(sfZKProof);
    if (confidentialFields && !checkConfidentialLendingDependencies(ctx.rules))
        return false;
    return checkLendingProtocolDependencies(ctx.rules, ctx.tx);
}

NotTEC
LoanBrokerSet::preflight(PreflightContext const& ctx)
{
    using namespace lending;

    auto const& tx = ctx.tx;
    if (auto const data = tx[~sfData];
        data && !data->empty() && !validDataLength(tx[~sfData], kMaxDataPayloadLength))
        return temINVALID;
    if (!validNumericRange(tx[~sfManagementFeeRate], kMaxManagementFeeRate))
        return temINVALID;
    if (!validNumericRange(tx[~sfCoverRateMinimum], kMaxCoverRate))
        return temINVALID;
    if (!validNumericRange(tx[~sfCoverRateLiquidation], kMaxCoverRate))
        return temINVALID;
    if (!validNumericRange(tx[~sfDebtMaximum], Number(kMaxMpTokenAmount), Number(0)))
        return temINVALID;

    if (tx.isFieldPresent(sfLoanBrokerID))
    {
        // We're modifying an existing LoanBroker.
        // Fixed fields can not be specified if we're modifying an existing
        // LoanBroker Object
        if (tx.isFieldPresent(sfManagementFeeRate) || tx.isFieldPresent(sfCoverRateMinimum) ||
            tx.isFieldPresent(sfCoverRateLiquidation))
            return temINVALID;

        if (tx[sfLoanBrokerID] == beast::kZero)
            return temINVALID;

        // Cannot change private flag on existing broker
        if (tx.isFlag(tfLoanBrokerPrivate))
        {
            return temINVALID;
        }

        // A LoanBroker can only be made confidential when it is created
        if (tx.isFlag(tfLoanBrokerConfidential))
        {
            return temINVALID;
        }
    }
    else
    {
        // We're creating a new LoanBroker.
        if (auto const domainID = tx[~sfDomainID])
        {
            if (*domainID == beast::kZero)
            {
                // DomainID must not be zero if provided
                return temMALFORMED;
            }
            if (!tx.isFlag(tfLoanBrokerPrivate))
            {
                // Public brokers cannot have a DomainID
                return temINVALID;
            }
        }
    }

    if (auto const vaultID = tx.at(~sfVaultID))
    {
        if (*vaultID == beast::kZero)
            return temINVALID;
    }

    {
        auto const minimumZero = tx[~sfCoverRateMinimum].value_or(0) == 0;
        auto const liquidationZero = tx[~sfCoverRateLiquidation].value_or(0) == 0;
        // Both must be zero or non-zero.
        if (minimumZero != liquidationZero)
        {
            return temINVALID;
        }
    }

    bool const hasKey = tx.isFieldPresent(sfHolderEncryptionKey);
    bool const hasProof = tx.isFieldPresent(sfZKProof);
    if (tx.isFlag(tfLoanBrokerConfidential))
    {
        // The Broker Key and its proof of knowledge are both required
        if (!hasKey || !hasProof)
            return temMALFORMED;

        if (!isValidCompressedECPoint(tx[sfHolderEncryptionKey]) ||
            tx[sfZKProof].size() != kEcSchnorrProofLength)
            return temMALFORMED;

        // A confidential LoanBroker has no management fee and no First-Loss
        // Capital
        if (tx[~sfManagementFeeRate].value_or(0) != 0 ||
            tx[~sfCoverRateMinimum].value_or(0) != 0 ||
            tx[~sfCoverRateLiquidation].value_or(0) != 0)
            return temINVALID;
    }
    else if (hasKey || hasProof)
    {
        return temMALFORMED;
    }

    return tesSUCCESS;
}

std::vector<OptionaledField<STNumber>> const&
LoanBrokerSet::getValueFields()
{
    static std::vector<OptionaledField<STNumber>> const kValueFields{~sfDebtMaximum};

    return kValueFields;
}

std::uint32_t
LoanBrokerSet::getFlagsMask(PreflightContext const& ctx)
{
    auto mask = tfLoanBrokerSetMask;
    if (!ctx.rules.enabled(featureLendingProtocolV1_2))
        mask |= tfLoanBrokerPrivate;
    if (!checkConfidentialLendingDependencies(ctx.rules))
        mask |= tfLoanBrokerConfidential;
    return mask;
}

XRPAmount
LoanBrokerSet::calculateBaseFee(ReadView const& view, STTx const& tx)
{
    if (tx.isFlag(tfLoanBrokerConfidential))
        return Transactor::calculateBaseFee(view, tx, kConfidentialFeeMultiplier);
    return Transactor::calculateBaseFee(view, tx);
}

namespace {

// A confidential LoanBroker needs a CashBasis Vault whose asset is a
// transferable MPT that can be held confidentially and has issuer and auditor
// keys, and a proof that the submitter knows the Broker Key.
TER
preclaimConfidential(PreclaimContext const& ctx, SLE::ConstRef sleVault, Asset const& asset)
{
    if (!asset.holds<MPTIssue>())
    {
        JLOG(ctx.j.warn()) << "Confidential LoanBroker requires an MPT Vault asset.";
        return tecNO_PERMISSION;
    }
    auto const issuanceID = asset.get<MPTIssue>().getMptID();

    auto const sleIssuance = ctx.view.read(keylet::mptokenIssuance(issuanceID));
    if (!sleIssuance || !sleIssuance->isFlag(lsfMPTCanHoldConfidentialBalance) ||
        !sleIssuance->isFlag(lsfMPTCanTransfer) ||
        !sleIssuance->isFieldPresent(sfIssuerEncryptionKey) ||
        !sleIssuance->isFieldPresent(sfAuditorEncryptionKey))
    {
        JLOG(ctx.j.warn()) << "Vault asset does not support confidential lending.";
        return tecNO_PERMISSION;
    }

    if (getVaultVersion(sleVault) != VaultVersion::CashBasis)
    {
        JLOG(ctx.j.warn()) << "Confidential LoanBroker requires a CashBasis Vault.";
        return tecNO_PERMISSION;
    }

    auto const contextHash = getLoanBrokerSetContextHash(
        ctx.tx[sfAccount], issuanceID, ctx.tx.getSeqProxy().value());
    if (!isTesSuccess(
            verifySchnorrProof(ctx.tx[sfHolderEncryptionKey], ctx.tx[sfZKProof], contextHash)))
    {
        JLOG(ctx.j.warn()) << "Invalid proof of knowledge for the Broker Key.";
        return tecBAD_PROOF;
    }

    return tesSUCCESS;
}

// Initialize the Lending Balance on the LoanBroker pseudo-account's MPToken:
// register the Broker Key and set every confidential balance to the canonical
// encrypted zero, computed for the pseudo-account.
TER
initLendingBalance(
    ApplyView& view,
    AccountID const& pseudoId,
    Asset const& asset,
    Slice const& brokerKey)
{
    auto const issuanceID = asset.get<MPTIssue>().getMptID();
    auto const sleIssuance = view.read(keylet::mptokenIssuance(issuanceID));
    auto const sleMptoken = view.peek(keylet::mptoken(issuanceID, pseudoId));
    if (!sleIssuance || !sleMptoken)
        return tecINTERNAL;  // LCOV_EXCL_LINE

    auto const zero = encryptCanonicalZeroAmount(brokerKey, pseudoId, issuanceID);
    auto const issuerZero =
        encryptCanonicalZeroAmount((*sleIssuance)[sfIssuerEncryptionKey], pseudoId, issuanceID);
    auto const auditorZero =
        encryptCanonicalZeroAmount((*sleIssuance)[sfAuditorEncryptionKey], pseudoId, issuanceID);
    if (!zero || !issuerZero || !auditorZero)
        return tecINTERNAL;  // LCOV_EXCL_LINE

    (*sleMptoken)[sfHolderEncryptionKey] = brokerKey;
    (*sleMptoken)[sfConfidentialBalanceSpending] = *zero;
    (*sleMptoken)[sfConfidentialBalanceInbox] = *zero;
    (*sleMptoken)[sfIssuerEncryptedBalance] = *issuerZero;
    (*sleMptoken)[sfAuditorEncryptedBalance] = *auditorZero;
    (*sleMptoken)[sfConfidentialBalanceVersion] = 0;
    view.update(sleMptoken);

    return tesSUCCESS;
}

}  // namespace

TER
LoanBrokerSet::preclaim(PreclaimContext const& ctx)
{
    auto const& tx = ctx.tx;

    auto const account = tx[sfAccount];
    auto const vaultID = tx[sfVaultID];

    auto const sleVault = ctx.view.read(keylet::vault(vaultID));
    if (!sleVault)
    {
        JLOG(ctx.j.warn()) << "Vault does not exist.";
        return tecNO_ENTRY;
    }
    Asset const asset = sleVault->at(sfAsset);

    if (account != sleVault->at(sfOwner))
    {
        JLOG(ctx.j.warn()) << "Account is not the owner of the Vault.";
        return tecNO_PERMISSION;
    }

    if (auto const brokerID = tx[~sfLoanBrokerID])
    {
        // Updating an existing Broker

        auto const sleBroker = ctx.view.read(keylet::loanBroker(*brokerID));
        if (!sleBroker)
        {
            JLOG(ctx.j.warn()) << "LoanBroker does not exist.";
            return tecNO_ENTRY;
        }
        if (vaultID != sleBroker->at(sfVaultID))
        {
            JLOG(ctx.j.warn()) << "Can not change VaultID on an existing LoanBroker.";
            return tecNO_PERMISSION;
        }
        if (account != sleBroker->at(sfOwner))
        {
            JLOG(ctx.j.warn()) << "Account is not the owner of the LoanBroker.";
            return tecNO_PERMISSION;
        }

        if (auto const debtMax = tx[~sfDebtMaximum])
        {
            // Can't reduce the debt maximum below the current total debt
            auto const currentDebtTotal = sleBroker->at(sfDebtTotal);
            if (*debtMax != 0 && *debtMax < currentDebtTotal)
            {
                JLOG(ctx.j.warn()) << "Cannot reduce DebtMaximum below current DebtTotal.";
                return tecLIMIT_EXCEEDED;
            }
        }

        if (!sleBroker->isFlag(lsfLoanBrokerPrivate) && tx.isFieldPresent(sfDomainID))
            return tecNO_PERMISSION;
    }
    else
    {
        // LP V1.1: only closed-ended vaults may host a loan broker. The
        // lending protocol relies on the closed-ended Subscription /
        // Investment / Redemption phase structure; attaching a broker to
        // an open-ended vault has no well-defined lifecycle. VaultCreate
        // stays unrestricted so existing open-ended flows keep working;
        // the constraint is enforced here, at the point where the vault
        // is first bound to the lending protocol.
        if (ctx.view.rules().enabled(featureLendingProtocolV1_1) &&
            getVaultKind(sleVault) != VaultKind::ClosedEnded)
        {
            JLOG(ctx.j.warn()) << "LoanBroker requires a closed-ended Vault.";
            return tecNO_PERMISSION;
        }

        if (auto const ter = canAddHolding(ctx.view, asset))
            return ter;

        if (auto const ter = checkFrozen(ctx.view, sleVault->at(sfAccount), sleVault->at(sfAsset)))
        {
            JLOG(ctx.j.warn()) << "Vault pseudo-account is frozen.";
            return ter;
        }

        if (tx.isFlag(tfLoanBrokerConfidential))
        {
            if (auto const ter = preclaimConfidential(ctx, sleVault, asset))
                return ter;
        }
    }

    if (auto const domainID = tx[~sfDomainID]; domainID && *domainID != beast::kZero)
    {
        if (!ctx.view.exists(keylet::permissionedDomain(*domainID)))
        {
            JLOG(ctx.j.warn()) << "Domain does not exist.";
            return tecOBJECT_NOT_FOUND;
        }
    }

    // Check that relevant values can be represented as the vault asset
    // type. This is mostly only relevant for integral (non-IOU) types.
    // fixCleanup3_5_0 also rejects out-of-range values, e.g. XRP above
    // STAmount::kMaxNativeN, which would otherwise throw.
    bool const fix350Enabled = ctx.view.rules().enabled(fixCleanup3_5_0);
    for (auto const& field : getValueFields())
    {
        if (auto const value = tx[field]; value &&
            ((fix350Enabled && !isRepresentable(asset, *value)) ||
             STAmount{asset, *value} != *value ||
             (getVaultVersion(sleVault) == VaultVersion::FixedPrecision &&
              !isOnVaultBaseGrid(asset, *value, getVaultBaseScale(sleVault)))))
        {
            JLOG(ctx.j.warn()) << field.f->getName() << " (" << *value
                               << ") can not be represented as a(n) " << to_string(asset) << ".";
            return tecPRECISION_LOSS;
        }
    }

    return tesSUCCESS;
}

TER
LoanBrokerSet::doApply()
{
    auto const& tx = ctx_.tx;
    auto& view = ctx_.view();

    if (auto const brokerID = tx[~sfLoanBrokerID])
    {
        // Modify an existing LoanBroker
        auto const broker = view.peek(keylet::loanBroker(*brokerID));
        if (!broker)
        {
            // This should be impossible
            // LCOV_EXCL_START
            JLOG(j_.fatal()) << "LoanBroker does not exist.";
            return tefBAD_LEDGER;
            // LCOV_EXCL_STOP
        }

        auto const vault = view.read(keylet::vault(broker->at(sfVaultID)));
        if (!vault)
            return tecINTERNAL;  // LCOV_EXCL_LINE

        auto const vaultAsset = vault->at(sfAsset);

        if (auto const data = tx[~sfData])
            broker->at(sfData) = *data;
        if (auto const debtMax = tx[~sfDebtMaximum])
            broker->at(sfDebtMaximum) = *debtMax;

        if (auto const domainID = tx[~sfDomainID])
        {
            if (*domainID != beast::kZero)
            {
                broker->setFieldH256(sfDomainID, *domainID);
            }
            else if (broker->isFieldPresent(sfDomainID))
            {
                broker->makeFieldAbsent(sfDomainID);
            }
        }

        view.update(broker);

        associateAsset(*broker, vaultAsset);
    }
    else
    {
        // Create a new LoanBroker pointing back to the given Vault
        auto const vaultID = tx[sfVaultID];
        auto const sleVault = view.read(keylet::vault(vaultID));
        if (!sleVault)
        {
            // This should be impossible
            // LCOV_EXCL_START
            JLOG(j_.fatal()) << "Vault does not exist.";
            return tefBAD_LEDGER;
            // LCOV_EXCL_STOP
        }
        auto const vaultPseudoID = sleVault->at(sfAccount);
        auto const vaultAsset = sleVault->at(sfAsset);
        auto const sequence = tx.getSeqProxy();

        auto const owner = view.peek(keylet::account(accountID_));
        if (!owner)
        {
            // This should be impossible
            // LCOV_EXCL_START
            JLOG(j_.fatal()) << "Account does not exist.";
            return tefBAD_LEDGER;
            // LCOV_EXCL_STOP
        }
        auto const broker = std::make_shared<SLE>(keylet::loanBroker(accountID_, sequence));

        if (auto const ter = dirLink(view, accountID_, broker))
            return ter;  // LCOV_EXCL_LINE
        if (auto const ter = dirLink(view, vaultPseudoID, broker, sfVaultNode))
            return ter;  // LCOV_EXCL_LINE

        // Increases the owner count by two: one for the LoanBroker object, and
        // one for the pseudo-account.
        increaseOwnerCount(view, owner, {}, 2, j_);
        if (preFeeBalance_ < accountReserve(view, owner, j_))
            return tecINSUFFICIENT_RESERVE;

        auto maybePseudo = createPseudoAccount(view, broker->key(), sfLoanBrokerID);
        if (!maybePseudo)
            return maybePseudo.error();  // LCOV_EXCL_LINE
        auto const& pseudo = *maybePseudo;
        AccountID const pseudoId = pseudo->at(sfAccount);

        if (auto ter = addEmptyHolding(
                ctx_.getApplyViewContext(), pseudoId, preFeeBalance_, sleVault->at(sfAsset), j_))
            return ter;

        // Initialize data fields:
        broker->at(sfSequence) = sequence.value();
        broker->at(sfVaultID) = vaultID;
        broker->at(sfOwner) = accountID_;
        broker->at(sfAccount) = pseudoId;
        // The LoanSequence indexes loans created by this broker, starting at 1
        broker->at(sfLoanSequence) = 1;
        if (auto const data = tx[~sfData])
            broker->at(sfData) = *data;
        if (auto const rate = tx[~sfManagementFeeRate])
            broker->at(sfManagementFeeRate) = *rate;
        if (auto const debtMax = tx[~sfDebtMaximum])
            broker->at(sfDebtMaximum) = *debtMax;
        if (auto const coverMin = tx[~sfCoverRateMinimum])
            broker->at(sfCoverRateMinimum) = *coverMin;
        if (auto const coverLiq = tx[~sfCoverRateLiquidation])
            broker->at(sfCoverRateLiquidation) = *coverLiq;

        if (tx.isFlag(tfLoanBrokerPrivate))
        {
            broker->setFlag(lsfLoanBrokerPrivate);
            if (auto const domainID = tx[~sfDomainID])
                broker->setFieldH256(sfDomainID, *domainID);
        }

        if (tx.isFlag(tfLoanBrokerConfidential))
        {
            broker->setFlag(lsfLoanBrokerConfidential);
            if (auto const ter =
                    initLendingBalance(view, pseudoId, vaultAsset, tx[sfHolderEncryptionKey]))
                return ter;
        }

        view.insert(broker);

        associateAsset(*broker, vaultAsset);
    }

    return tesSUCCESS;
}

void
LoanBrokerSet::visitInvariantEntry(bool, SLE::ConstRef, SLE::ConstRef)
{
    // No transaction-specific invariants yet (future work).
}

bool
LoanBrokerSet::finalizeInvariants(
    STTx const&,
    TER,
    XRPAmount,
    ReadView const&,
    beast::Journal const&)
{
    // No transaction-specific invariants yet (future work).
    return true;
}

//------------------------------------------------------------------------------

}  // namespace xrpl
