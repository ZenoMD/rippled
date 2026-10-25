#pragma once

#include <xrpl/beast/utility/Journal.h>
#include <xrpl/core/ServiceRegistry.h>
#include <xrpl/ledger/ReadView.h>
#include <xrpl/protocol/AccountID.h>
#include <xrpl/protocol/SField.h>
#include <xrpl/protocol/STNumber.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/TER.h>
#include <xrpl/protocol/XRPAmount.h>
#include <xrpl/tx/ApplyContext.h>
#include <xrpl/tx/Transactor.h>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace xrpl {

class LoanSet : public Transactor
{
public:
    static constexpr auto kConsequencesFactory = ConsequencesFactoryType::Normal;

    explicit LoanSet(ApplyContext& ctx) : Transactor(ctx)
    {
    }

    static bool
    checkExtraFeatures(PreflightContext const& ctx);

    static std::uint32_t
    getFlagsMask(PreflightContext const& ctx);

    static NotTEC
    preflight(PreflightContext const& ctx);

    static NotTEC
    checkSign(PreclaimContext const& ctx);

    static XRPAmount
    calculateBaseFee(ReadView const& view, STTx const& tx);

    static std::vector<OptionaledField<STNumber>> const&
    getValueFields();

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

public:
    static constexpr std::uint32_t kMinPaymentTotal = 1;
    static constexpr std::uint32_t kDefaultPaymentTotal = 1;
    static_assert(kDefaultPaymentTotal >= kMinPaymentTotal);

    static constexpr std::uint32_t kMinPaymentInterval = 60;
    static constexpr std::uint32_t kDefaultPaymentInterval = 60;
    static_assert(kDefaultPaymentInterval >= kMinPaymentInterval);

    static constexpr std::uint32_t kDefaultGracePeriod = 60;
    static_assert(kDefaultGracePeriod >= kMinPaymentInterval);

    /** Size of a confidential LoanSet's ZKProof (spec §11.2). */
    static constexpr std::size_t kConfidentialProofLength = 3030;

    /** True if the transaction carries the confidential Loan fields. */
    static bool
    isConfidential(STTx const& tx);

private:
    static TER
    preclaimConfidential(
        PreclaimContext const& ctx,
        SLE::ConstRef brokerSle,
        SLE::ConstRef vault,
        AccountID const& borrower);

    TER
    doApplyConfidential(SLE::Ref brokerSle, SLE::Ref borrowerSle, AccountID const& borrower);

    // Collected by visitInvariantEntry for finalizeInvariants
    bool vaultModified_ = false;
    bool brokerDebtChanged_ = false;
    bool supplyChanged_ = false;
};

//------------------------------------------------------------------------------

}  // namespace xrpl
