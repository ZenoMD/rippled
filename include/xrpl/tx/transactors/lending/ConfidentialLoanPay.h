#pragma once

#include <xrpl/beast/utility/Journal.h>
#include <xrpl/core/ServiceRegistry.h>
#include <xrpl/ledger/ReadView.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/TER.h>
#include <xrpl/protocol/XRPAmount.h>
#include <xrpl/tx/ApplyContext.h>
#include <xrpl/tx/Transactor.h>

#include <cstddef>
#include <cstdint>

namespace xrpl {

/**
 * The Borrower pays a confidential Loan from their confidential spending
 * balance into the Lending Balance's inbox. A regular payment is at least the
 * Periodic Payment and at most the Amount Owed; a full payment
 * (tfLoanFullPayment) is exactly the Amount Owed and closes the Loan.
 */
class ConfidentialLoanPay : public Transactor
{
public:
    static constexpr auto kConsequencesFactory = ConsequencesFactoryType::Normal;

    /** ZKProof sizes (spec §13.3) */
    static constexpr std::size_t kRegularProofLength = 2148;
    static constexpr std::size_t kFullProofLength = 1202;

    explicit ConfidentialLoanPay(ApplyContext& ctx) : Transactor(ctx)
    {
    }

    static bool
    checkExtraFeatures(PreflightContext const& ctx);

    static std::uint32_t
    getFlagsMask(PreflightContext const& ctx);

    static NotTEC
    preflight(PreflightContext const& ctx);

    static XRPAmount
    calculateBaseFee(ReadView const& view, STTx const& tx);

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
    // Collected by visitInvariantEntry for finalizeInvariants
    bool vaultModified_ = false;
    bool brokerDebtChanged_ = false;
};

//------------------------------------------------------------------------------

}  // namespace xrpl
