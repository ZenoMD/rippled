#pragma once

#include <xrpl/basics/Number.h>
#include <xrpl/beast/utility/Journal.h>
#include <xrpl/core/ServiceRegistry.h>
#include <xrpl/ledger/ReadView.h>
#include <xrpl/protocol/AccountID.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/TER.h>
#include <xrpl/protocol/XRPAmount.h>
#include <xrpl/tx/ApplyContext.h>
#include <xrpl/tx/Transactor.h>

#include <cstdint>
#include <map>
#include <optional>

namespace xrpl {

/**
 * Returns funds from the Lending Balance of a confidential LoanBroker to its
 * Vault. The amount is public. It works like a ConfidentialMPTConvertBack
 * with the Vault pseudo-account's public balance as the destination.
 */
class LoanBrokerSettle : public Transactor
{
public:
    static constexpr auto kConsequencesFactory = ConsequencesFactoryType::Normal;

    explicit LoanBrokerSettle(ApplyContext& ctx) : Transactor(ctx)
    {
    }

    static bool
    checkExtraFeatures(PreflightContext const& ctx);

    static NotTEC
    preflight(PreflightContext const& ctx);

    static TER
    preclaim(PreclaimContext const& ctx);

    TER
    doApply() override;

    void
    visitInvariantEntry(bool isDelete, SLE::ConstRef before, SLE::ConstRef after) override;

    [[nodiscard]] bool
    finalizeInvariants(
        STTx const& tx,
        TER result,
        XRPAmount fee,
        ReadView const& view,
        beast::Journal const& j) override;

private:
    // Before and after values of the fields the settlement must move, collected by
    // visitInvariantEntry and checked by finalizeInvariants.
    template <class T>
    struct Change
    {
        std::optional<T> before;
        std::optional<T> after;
    };

    Change<Number> assetsAvailable_;
    Change<Number> assetsTotal_;
    Change<Number> debtTotal_;
    Change<std::uint64_t> confidentialOutstanding_;
    Change<std::uint64_t> outstanding_;
    std::optional<AccountID> vaultPseudo_;
    std::map<AccountID, Change<std::uint64_t>> mptAmounts_;
};

//------------------------------------------------------------------------------

}  // namespace xrpl
