// Auto-generated unit tests for transaction LoanSet


#include <gtest/gtest.h>

#include <protocol_autogen/TestHelpers.h>

#include <xrpl/protocol/SecretKey.h>
#include <xrpl/protocol/Seed.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol_autogen/transactions/LoanSet.h>
#include <xrpl/protocol_autogen/transactions/AccountSet.h>

#include <string>

namespace xrpl::transactions {

// 1 & 4) Set fields via builder setters, build, then read them back via
// wrapper getters. After build(), validate() should succeed.
TEST(TransactionsLoanSetTests, BuilderSettersRoundTrip)
{
    // Generate a deterministic keypair for signing
    auto const [publicKey, secretKey] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testLoanSet"));

    // Common transaction fields
    auto const accountValue = calcAccountID(publicKey);
    std::uint32_t const sequenceValue = 1;
    auto const feeValue = canonical_AMOUNT();

    // Transaction-specific field values
    auto const loanBrokerIDValue = canonical_UINT256();
    auto const dataValue = canonical_VL();
    auto const counterpartyValue = canonical_ACCOUNT();
    auto const counterpartySignatureValue = canonical_OBJECT();
    auto const loanOriginationFeeValue = canonical_NUMBER();
    auto const loanServiceFeeValue = canonical_NUMBER();
    auto const latePaymentFeeValue = canonical_NUMBER();
    auto const closePaymentFeeValue = canonical_NUMBER();
    auto const overpaymentFeeValue = canonical_UINT32();
    auto const interestRateValue = canonical_UINT32();
    auto const lateInterestRateValue = canonical_UINT32();
    auto const closeInterestRateValue = canonical_UINT32();
    auto const overpaymentInterestRateValue = canonical_UINT32();
    auto const principalRequestedValue = canonical_NUMBER();
    auto const paymentTotalValue = canonical_UINT32();
    auto const paymentIntervalValue = canonical_UINT32();
    auto const gracePeriodValue = canonical_UINT32();
    auto const senderEncryptedAmountValue = canonical_VL();
    auto const destinationEncryptedAmountValue = canonical_VL();
    auto const issuerEncryptedAmountValue = canonical_VL();
    auto const auditorEncryptedAmountValue = canonical_VL();
    auto const balanceCommitmentValue = canonical_VL();
    auto const amountCommitmentValue = canonical_VL();
    auto const zKProofValue = canonical_VL();
    auto const loanAuditorEncryptionKeyValue = canonical_VL();
    auto const loanAuditorEncryptedPrincipalValue = canonical_VL();
    auto const borrowerEncryptedAmountOwedValue = canonical_VL();
    auto const brokerEncryptedAmountOwedValue = canonical_VL();
    auto const loanAuditorEncryptedAmountOwedValue = canonical_VL();
    auto const borrowerEncryptedPeriodicPaymentValue = canonical_VL();
    auto const brokerEncryptedPeriodicPaymentValue = canonical_VL();
    auto const loanAuditorEncryptedPeriodicPaymentValue = canonical_VL();
    auto const amountOwedCommitmentValue = canonical_VL();
    auto const periodicPaymentCommitmentValue = canonical_VL();

    LoanSetBuilder builder{
        accountValue,
        loanBrokerIDValue,
        sequenceValue,
        feeValue
    };

    // Set optional fields
    builder.setData(dataValue);
    builder.setCounterparty(counterpartyValue);
    builder.setCounterpartySignature(counterpartySignatureValue);
    builder.setLoanOriginationFee(loanOriginationFeeValue);
    builder.setLoanServiceFee(loanServiceFeeValue);
    builder.setLatePaymentFee(latePaymentFeeValue);
    builder.setClosePaymentFee(closePaymentFeeValue);
    builder.setOverpaymentFee(overpaymentFeeValue);
    builder.setInterestRate(interestRateValue);
    builder.setLateInterestRate(lateInterestRateValue);
    builder.setCloseInterestRate(closeInterestRateValue);
    builder.setOverpaymentInterestRate(overpaymentInterestRateValue);
    builder.setPrincipalRequested(principalRequestedValue);
    builder.setPaymentTotal(paymentTotalValue);
    builder.setPaymentInterval(paymentIntervalValue);
    builder.setGracePeriod(gracePeriodValue);
    builder.setSenderEncryptedAmount(senderEncryptedAmountValue);
    builder.setDestinationEncryptedAmount(destinationEncryptedAmountValue);
    builder.setIssuerEncryptedAmount(issuerEncryptedAmountValue);
    builder.setAuditorEncryptedAmount(auditorEncryptedAmountValue);
    builder.setBalanceCommitment(balanceCommitmentValue);
    builder.setAmountCommitment(amountCommitmentValue);
    builder.setZKProof(zKProofValue);
    builder.setLoanAuditorEncryptionKey(loanAuditorEncryptionKeyValue);
    builder.setLoanAuditorEncryptedPrincipal(loanAuditorEncryptedPrincipalValue);
    builder.setBorrowerEncryptedAmountOwed(borrowerEncryptedAmountOwedValue);
    builder.setBrokerEncryptedAmountOwed(brokerEncryptedAmountOwedValue);
    builder.setLoanAuditorEncryptedAmountOwed(loanAuditorEncryptedAmountOwedValue);
    builder.setBorrowerEncryptedPeriodicPayment(borrowerEncryptedPeriodicPaymentValue);
    builder.setBrokerEncryptedPeriodicPayment(brokerEncryptedPeriodicPaymentValue);
    builder.setLoanAuditorEncryptedPeriodicPayment(loanAuditorEncryptedPeriodicPaymentValue);
    builder.setAmountOwedCommitment(amountOwedCommitmentValue);
    builder.setPeriodicPaymentCommitment(periodicPaymentCommitmentValue);

    auto tx = builder.build(publicKey, secretKey);

    std::string reason;
    EXPECT_TRUE(tx.validate(reason)) << reason;

    // Verify signing was applied
    EXPECT_FALSE(tx.getSigningPubKey().empty());
    EXPECT_TRUE(tx.hasTxnSignature());

    // Verify common fields
    EXPECT_EQ(tx.getAccount(), accountValue);
    EXPECT_EQ(tx.getSequence(), sequenceValue);
    EXPECT_EQ(tx.getFee(), feeValue);

    // Verify required fields
    {
        auto const& expected = loanBrokerIDValue;
        auto const actual = tx.getLoanBrokerID();
        expectEqualField(expected, actual, "sfLoanBrokerID");
    }

    // Verify optional fields
    {
        auto const& expected = dataValue;
        auto const actualOpt = tx.getData();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfData should be present";
        expectEqualField(expected, *actualOpt, "sfData");
        EXPECT_TRUE(tx.hasData());
    }

    {
        auto const& expected = counterpartyValue;
        auto const actualOpt = tx.getCounterparty();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfCounterparty should be present";
        expectEqualField(expected, *actualOpt, "sfCounterparty");
        EXPECT_TRUE(tx.hasCounterparty());
    }

    {
        auto const& expected = counterpartySignatureValue;
        auto const actualOpt = tx.getCounterpartySignature();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfCounterpartySignature should be present";
        expectEqualField(expected, *actualOpt, "sfCounterpartySignature");
        EXPECT_TRUE(tx.hasCounterpartySignature());
    }

    {
        auto const& expected = loanOriginationFeeValue;
        auto const actualOpt = tx.getLoanOriginationFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanOriginationFee should be present";
        expectEqualField(expected, *actualOpt, "sfLoanOriginationFee");
        EXPECT_TRUE(tx.hasLoanOriginationFee());
    }

    {
        auto const& expected = loanServiceFeeValue;
        auto const actualOpt = tx.getLoanServiceFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanServiceFee should be present";
        expectEqualField(expected, *actualOpt, "sfLoanServiceFee");
        EXPECT_TRUE(tx.hasLoanServiceFee());
    }

    {
        auto const& expected = latePaymentFeeValue;
        auto const actualOpt = tx.getLatePaymentFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLatePaymentFee should be present";
        expectEqualField(expected, *actualOpt, "sfLatePaymentFee");
        EXPECT_TRUE(tx.hasLatePaymentFee());
    }

    {
        auto const& expected = closePaymentFeeValue;
        auto const actualOpt = tx.getClosePaymentFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfClosePaymentFee should be present";
        expectEqualField(expected, *actualOpt, "sfClosePaymentFee");
        EXPECT_TRUE(tx.hasClosePaymentFee());
    }

    {
        auto const& expected = overpaymentFeeValue;
        auto const actualOpt = tx.getOverpaymentFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfOverpaymentFee should be present";
        expectEqualField(expected, *actualOpt, "sfOverpaymentFee");
        EXPECT_TRUE(tx.hasOverpaymentFee());
    }

    {
        auto const& expected = interestRateValue;
        auto const actualOpt = tx.getInterestRate();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfInterestRate should be present";
        expectEqualField(expected, *actualOpt, "sfInterestRate");
        EXPECT_TRUE(tx.hasInterestRate());
    }

    {
        auto const& expected = lateInterestRateValue;
        auto const actualOpt = tx.getLateInterestRate();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLateInterestRate should be present";
        expectEqualField(expected, *actualOpt, "sfLateInterestRate");
        EXPECT_TRUE(tx.hasLateInterestRate());
    }

    {
        auto const& expected = closeInterestRateValue;
        auto const actualOpt = tx.getCloseInterestRate();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfCloseInterestRate should be present";
        expectEqualField(expected, *actualOpt, "sfCloseInterestRate");
        EXPECT_TRUE(tx.hasCloseInterestRate());
    }

    {
        auto const& expected = overpaymentInterestRateValue;
        auto const actualOpt = tx.getOverpaymentInterestRate();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfOverpaymentInterestRate should be present";
        expectEqualField(expected, *actualOpt, "sfOverpaymentInterestRate");
        EXPECT_TRUE(tx.hasOverpaymentInterestRate());
    }

    {
        auto const& expected = principalRequestedValue;
        auto const actualOpt = tx.getPrincipalRequested();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfPrincipalRequested should be present";
        expectEqualField(expected, *actualOpt, "sfPrincipalRequested");
        EXPECT_TRUE(tx.hasPrincipalRequested());
    }

    {
        auto const& expected = paymentTotalValue;
        auto const actualOpt = tx.getPaymentTotal();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfPaymentTotal should be present";
        expectEqualField(expected, *actualOpt, "sfPaymentTotal");
        EXPECT_TRUE(tx.hasPaymentTotal());
    }

    {
        auto const& expected = paymentIntervalValue;
        auto const actualOpt = tx.getPaymentInterval();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfPaymentInterval should be present";
        expectEqualField(expected, *actualOpt, "sfPaymentInterval");
        EXPECT_TRUE(tx.hasPaymentInterval());
    }

    {
        auto const& expected = gracePeriodValue;
        auto const actualOpt = tx.getGracePeriod();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfGracePeriod should be present";
        expectEqualField(expected, *actualOpt, "sfGracePeriod");
        EXPECT_TRUE(tx.hasGracePeriod());
    }

    {
        auto const& expected = senderEncryptedAmountValue;
        auto const actualOpt = tx.getSenderEncryptedAmount();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfSenderEncryptedAmount should be present";
        expectEqualField(expected, *actualOpt, "sfSenderEncryptedAmount");
        EXPECT_TRUE(tx.hasSenderEncryptedAmount());
    }

    {
        auto const& expected = destinationEncryptedAmountValue;
        auto const actualOpt = tx.getDestinationEncryptedAmount();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfDestinationEncryptedAmount should be present";
        expectEqualField(expected, *actualOpt, "sfDestinationEncryptedAmount");
        EXPECT_TRUE(tx.hasDestinationEncryptedAmount());
    }

    {
        auto const& expected = issuerEncryptedAmountValue;
        auto const actualOpt = tx.getIssuerEncryptedAmount();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfIssuerEncryptedAmount should be present";
        expectEqualField(expected, *actualOpt, "sfIssuerEncryptedAmount");
        EXPECT_TRUE(tx.hasIssuerEncryptedAmount());
    }

    {
        auto const& expected = auditorEncryptedAmountValue;
        auto const actualOpt = tx.getAuditorEncryptedAmount();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfAuditorEncryptedAmount should be present";
        expectEqualField(expected, *actualOpt, "sfAuditorEncryptedAmount");
        EXPECT_TRUE(tx.hasAuditorEncryptedAmount());
    }

    {
        auto const& expected = balanceCommitmentValue;
        auto const actualOpt = tx.getBalanceCommitment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBalanceCommitment should be present";
        expectEqualField(expected, *actualOpt, "sfBalanceCommitment");
        EXPECT_TRUE(tx.hasBalanceCommitment());
    }

    {
        auto const& expected = amountCommitmentValue;
        auto const actualOpt = tx.getAmountCommitment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfAmountCommitment should be present";
        expectEqualField(expected, *actualOpt, "sfAmountCommitment");
        EXPECT_TRUE(tx.hasAmountCommitment());
    }

    {
        auto const& expected = zKProofValue;
        auto const actualOpt = tx.getZKProof();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfZKProof should be present";
        expectEqualField(expected, *actualOpt, "sfZKProof");
        EXPECT_TRUE(tx.hasZKProof());
    }

    {
        auto const& expected = loanAuditorEncryptionKeyValue;
        auto const actualOpt = tx.getLoanAuditorEncryptionKey();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanAuditorEncryptionKey should be present";
        expectEqualField(expected, *actualOpt, "sfLoanAuditorEncryptionKey");
        EXPECT_TRUE(tx.hasLoanAuditorEncryptionKey());
    }

    {
        auto const& expected = loanAuditorEncryptedPrincipalValue;
        auto const actualOpt = tx.getLoanAuditorEncryptedPrincipal();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanAuditorEncryptedPrincipal should be present";
        expectEqualField(expected, *actualOpt, "sfLoanAuditorEncryptedPrincipal");
        EXPECT_TRUE(tx.hasLoanAuditorEncryptedPrincipal());
    }

    {
        auto const& expected = borrowerEncryptedAmountOwedValue;
        auto const actualOpt = tx.getBorrowerEncryptedAmountOwed();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBorrowerEncryptedAmountOwed should be present";
        expectEqualField(expected, *actualOpt, "sfBorrowerEncryptedAmountOwed");
        EXPECT_TRUE(tx.hasBorrowerEncryptedAmountOwed());
    }

    {
        auto const& expected = brokerEncryptedAmountOwedValue;
        auto const actualOpt = tx.getBrokerEncryptedAmountOwed();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBrokerEncryptedAmountOwed should be present";
        expectEqualField(expected, *actualOpt, "sfBrokerEncryptedAmountOwed");
        EXPECT_TRUE(tx.hasBrokerEncryptedAmountOwed());
    }

    {
        auto const& expected = loanAuditorEncryptedAmountOwedValue;
        auto const actualOpt = tx.getLoanAuditorEncryptedAmountOwed();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanAuditorEncryptedAmountOwed should be present";
        expectEqualField(expected, *actualOpt, "sfLoanAuditorEncryptedAmountOwed");
        EXPECT_TRUE(tx.hasLoanAuditorEncryptedAmountOwed());
    }

    {
        auto const& expected = borrowerEncryptedPeriodicPaymentValue;
        auto const actualOpt = tx.getBorrowerEncryptedPeriodicPayment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBorrowerEncryptedPeriodicPayment should be present";
        expectEqualField(expected, *actualOpt, "sfBorrowerEncryptedPeriodicPayment");
        EXPECT_TRUE(tx.hasBorrowerEncryptedPeriodicPayment());
    }

    {
        auto const& expected = brokerEncryptedPeriodicPaymentValue;
        auto const actualOpt = tx.getBrokerEncryptedPeriodicPayment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBrokerEncryptedPeriodicPayment should be present";
        expectEqualField(expected, *actualOpt, "sfBrokerEncryptedPeriodicPayment");
        EXPECT_TRUE(tx.hasBrokerEncryptedPeriodicPayment());
    }

    {
        auto const& expected = loanAuditorEncryptedPeriodicPaymentValue;
        auto const actualOpt = tx.getLoanAuditorEncryptedPeriodicPayment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanAuditorEncryptedPeriodicPayment should be present";
        expectEqualField(expected, *actualOpt, "sfLoanAuditorEncryptedPeriodicPayment");
        EXPECT_TRUE(tx.hasLoanAuditorEncryptedPeriodicPayment());
    }

    {
        auto const& expected = amountOwedCommitmentValue;
        auto const actualOpt = tx.getAmountOwedCommitment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfAmountOwedCommitment should be present";
        expectEqualField(expected, *actualOpt, "sfAmountOwedCommitment");
        EXPECT_TRUE(tx.hasAmountOwedCommitment());
    }

    {
        auto const& expected = periodicPaymentCommitmentValue;
        auto const actualOpt = tx.getPeriodicPaymentCommitment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfPeriodicPaymentCommitment should be present";
        expectEqualField(expected, *actualOpt, "sfPeriodicPaymentCommitment");
        EXPECT_TRUE(tx.hasPeriodicPaymentCommitment());
    }

}

// 2 & 4) Start from an STTx, construct a builder from it, build a new wrapper,
// and verify all fields match.
TEST(TransactionsLoanSetTests, BuilderFromStTxRoundTrip)
{
    // Generate a deterministic keypair for signing
    auto const [publicKey, secretKey] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testLoanSetFromTx"));

    // Common transaction fields
    auto const accountValue = calcAccountID(publicKey);
    std::uint32_t const sequenceValue = 2;
    auto const feeValue = canonical_AMOUNT();

    // Transaction-specific field values
    auto const loanBrokerIDValue = canonical_UINT256();
    auto const dataValue = canonical_VL();
    auto const counterpartyValue = canonical_ACCOUNT();
    auto const counterpartySignatureValue = canonical_OBJECT();
    auto const loanOriginationFeeValue = canonical_NUMBER();
    auto const loanServiceFeeValue = canonical_NUMBER();
    auto const latePaymentFeeValue = canonical_NUMBER();
    auto const closePaymentFeeValue = canonical_NUMBER();
    auto const overpaymentFeeValue = canonical_UINT32();
    auto const interestRateValue = canonical_UINT32();
    auto const lateInterestRateValue = canonical_UINT32();
    auto const closeInterestRateValue = canonical_UINT32();
    auto const overpaymentInterestRateValue = canonical_UINT32();
    auto const principalRequestedValue = canonical_NUMBER();
    auto const paymentTotalValue = canonical_UINT32();
    auto const paymentIntervalValue = canonical_UINT32();
    auto const gracePeriodValue = canonical_UINT32();
    auto const senderEncryptedAmountValue = canonical_VL();
    auto const destinationEncryptedAmountValue = canonical_VL();
    auto const issuerEncryptedAmountValue = canonical_VL();
    auto const auditorEncryptedAmountValue = canonical_VL();
    auto const balanceCommitmentValue = canonical_VL();
    auto const amountCommitmentValue = canonical_VL();
    auto const zKProofValue = canonical_VL();
    auto const loanAuditorEncryptionKeyValue = canonical_VL();
    auto const loanAuditorEncryptedPrincipalValue = canonical_VL();
    auto const borrowerEncryptedAmountOwedValue = canonical_VL();
    auto const brokerEncryptedAmountOwedValue = canonical_VL();
    auto const loanAuditorEncryptedAmountOwedValue = canonical_VL();
    auto const borrowerEncryptedPeriodicPaymentValue = canonical_VL();
    auto const brokerEncryptedPeriodicPaymentValue = canonical_VL();
    auto const loanAuditorEncryptedPeriodicPaymentValue = canonical_VL();
    auto const amountOwedCommitmentValue = canonical_VL();
    auto const periodicPaymentCommitmentValue = canonical_VL();

    // Build an initial transaction
    LoanSetBuilder initialBuilder{
        accountValue,
        loanBrokerIDValue,
        sequenceValue,
        feeValue
    };

    initialBuilder.setData(dataValue);
    initialBuilder.setCounterparty(counterpartyValue);
    initialBuilder.setCounterpartySignature(counterpartySignatureValue);
    initialBuilder.setLoanOriginationFee(loanOriginationFeeValue);
    initialBuilder.setLoanServiceFee(loanServiceFeeValue);
    initialBuilder.setLatePaymentFee(latePaymentFeeValue);
    initialBuilder.setClosePaymentFee(closePaymentFeeValue);
    initialBuilder.setOverpaymentFee(overpaymentFeeValue);
    initialBuilder.setInterestRate(interestRateValue);
    initialBuilder.setLateInterestRate(lateInterestRateValue);
    initialBuilder.setCloseInterestRate(closeInterestRateValue);
    initialBuilder.setOverpaymentInterestRate(overpaymentInterestRateValue);
    initialBuilder.setPrincipalRequested(principalRequestedValue);
    initialBuilder.setPaymentTotal(paymentTotalValue);
    initialBuilder.setPaymentInterval(paymentIntervalValue);
    initialBuilder.setGracePeriod(gracePeriodValue);
    initialBuilder.setSenderEncryptedAmount(senderEncryptedAmountValue);
    initialBuilder.setDestinationEncryptedAmount(destinationEncryptedAmountValue);
    initialBuilder.setIssuerEncryptedAmount(issuerEncryptedAmountValue);
    initialBuilder.setAuditorEncryptedAmount(auditorEncryptedAmountValue);
    initialBuilder.setBalanceCommitment(balanceCommitmentValue);
    initialBuilder.setAmountCommitment(amountCommitmentValue);
    initialBuilder.setZKProof(zKProofValue);
    initialBuilder.setLoanAuditorEncryptionKey(loanAuditorEncryptionKeyValue);
    initialBuilder.setLoanAuditorEncryptedPrincipal(loanAuditorEncryptedPrincipalValue);
    initialBuilder.setBorrowerEncryptedAmountOwed(borrowerEncryptedAmountOwedValue);
    initialBuilder.setBrokerEncryptedAmountOwed(brokerEncryptedAmountOwedValue);
    initialBuilder.setLoanAuditorEncryptedAmountOwed(loanAuditorEncryptedAmountOwedValue);
    initialBuilder.setBorrowerEncryptedPeriodicPayment(borrowerEncryptedPeriodicPaymentValue);
    initialBuilder.setBrokerEncryptedPeriodicPayment(brokerEncryptedPeriodicPaymentValue);
    initialBuilder.setLoanAuditorEncryptedPeriodicPayment(loanAuditorEncryptedPeriodicPaymentValue);
    initialBuilder.setAmountOwedCommitment(amountOwedCommitmentValue);
    initialBuilder.setPeriodicPaymentCommitment(periodicPaymentCommitmentValue);

    auto initialTx = initialBuilder.build(publicKey, secretKey);

    // Create builder from existing STTx
    LoanSetBuilder builderFromTx{initialTx.getSTTx()};

    auto rebuiltTx = builderFromTx.build(publicKey, secretKey);

    std::string reason;
    EXPECT_TRUE(rebuiltTx.validate(reason)) << reason;

    // Verify common fields
    EXPECT_EQ(rebuiltTx.getAccount(), accountValue);
    EXPECT_EQ(rebuiltTx.getSequence(), sequenceValue);
    EXPECT_EQ(rebuiltTx.getFee(), feeValue);

    // Verify required fields
    {
        auto const& expected = loanBrokerIDValue;
        auto const actual = rebuiltTx.getLoanBrokerID();
        expectEqualField(expected, actual, "sfLoanBrokerID");
    }

    // Verify optional fields
    {
        auto const& expected = dataValue;
        auto const actualOpt = rebuiltTx.getData();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfData should be present";
        expectEqualField(expected, *actualOpt, "sfData");
    }

    {
        auto const& expected = counterpartyValue;
        auto const actualOpt = rebuiltTx.getCounterparty();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfCounterparty should be present";
        expectEqualField(expected, *actualOpt, "sfCounterparty");
    }

    {
        auto const& expected = counterpartySignatureValue;
        auto const actualOpt = rebuiltTx.getCounterpartySignature();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfCounterpartySignature should be present";
        expectEqualField(expected, *actualOpt, "sfCounterpartySignature");
    }

    {
        auto const& expected = loanOriginationFeeValue;
        auto const actualOpt = rebuiltTx.getLoanOriginationFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanOriginationFee should be present";
        expectEqualField(expected, *actualOpt, "sfLoanOriginationFee");
    }

    {
        auto const& expected = loanServiceFeeValue;
        auto const actualOpt = rebuiltTx.getLoanServiceFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanServiceFee should be present";
        expectEqualField(expected, *actualOpt, "sfLoanServiceFee");
    }

    {
        auto const& expected = latePaymentFeeValue;
        auto const actualOpt = rebuiltTx.getLatePaymentFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLatePaymentFee should be present";
        expectEqualField(expected, *actualOpt, "sfLatePaymentFee");
    }

    {
        auto const& expected = closePaymentFeeValue;
        auto const actualOpt = rebuiltTx.getClosePaymentFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfClosePaymentFee should be present";
        expectEqualField(expected, *actualOpt, "sfClosePaymentFee");
    }

    {
        auto const& expected = overpaymentFeeValue;
        auto const actualOpt = rebuiltTx.getOverpaymentFee();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfOverpaymentFee should be present";
        expectEqualField(expected, *actualOpt, "sfOverpaymentFee");
    }

    {
        auto const& expected = interestRateValue;
        auto const actualOpt = rebuiltTx.getInterestRate();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfInterestRate should be present";
        expectEqualField(expected, *actualOpt, "sfInterestRate");
    }

    {
        auto const& expected = lateInterestRateValue;
        auto const actualOpt = rebuiltTx.getLateInterestRate();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLateInterestRate should be present";
        expectEqualField(expected, *actualOpt, "sfLateInterestRate");
    }

    {
        auto const& expected = closeInterestRateValue;
        auto const actualOpt = rebuiltTx.getCloseInterestRate();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfCloseInterestRate should be present";
        expectEqualField(expected, *actualOpt, "sfCloseInterestRate");
    }

    {
        auto const& expected = overpaymentInterestRateValue;
        auto const actualOpt = rebuiltTx.getOverpaymentInterestRate();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfOverpaymentInterestRate should be present";
        expectEqualField(expected, *actualOpt, "sfOverpaymentInterestRate");
    }

    {
        auto const& expected = principalRequestedValue;
        auto const actualOpt = rebuiltTx.getPrincipalRequested();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfPrincipalRequested should be present";
        expectEqualField(expected, *actualOpt, "sfPrincipalRequested");
    }

    {
        auto const& expected = paymentTotalValue;
        auto const actualOpt = rebuiltTx.getPaymentTotal();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfPaymentTotal should be present";
        expectEqualField(expected, *actualOpt, "sfPaymentTotal");
    }

    {
        auto const& expected = paymentIntervalValue;
        auto const actualOpt = rebuiltTx.getPaymentInterval();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfPaymentInterval should be present";
        expectEqualField(expected, *actualOpt, "sfPaymentInterval");
    }

    {
        auto const& expected = gracePeriodValue;
        auto const actualOpt = rebuiltTx.getGracePeriod();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfGracePeriod should be present";
        expectEqualField(expected, *actualOpt, "sfGracePeriod");
    }

    {
        auto const& expected = senderEncryptedAmountValue;
        auto const actualOpt = rebuiltTx.getSenderEncryptedAmount();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfSenderEncryptedAmount should be present";
        expectEqualField(expected, *actualOpt, "sfSenderEncryptedAmount");
    }

    {
        auto const& expected = destinationEncryptedAmountValue;
        auto const actualOpt = rebuiltTx.getDestinationEncryptedAmount();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfDestinationEncryptedAmount should be present";
        expectEqualField(expected, *actualOpt, "sfDestinationEncryptedAmount");
    }

    {
        auto const& expected = issuerEncryptedAmountValue;
        auto const actualOpt = rebuiltTx.getIssuerEncryptedAmount();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfIssuerEncryptedAmount should be present";
        expectEqualField(expected, *actualOpt, "sfIssuerEncryptedAmount");
    }

    {
        auto const& expected = auditorEncryptedAmountValue;
        auto const actualOpt = rebuiltTx.getAuditorEncryptedAmount();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfAuditorEncryptedAmount should be present";
        expectEqualField(expected, *actualOpt, "sfAuditorEncryptedAmount");
    }

    {
        auto const& expected = balanceCommitmentValue;
        auto const actualOpt = rebuiltTx.getBalanceCommitment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBalanceCommitment should be present";
        expectEqualField(expected, *actualOpt, "sfBalanceCommitment");
    }

    {
        auto const& expected = amountCommitmentValue;
        auto const actualOpt = rebuiltTx.getAmountCommitment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfAmountCommitment should be present";
        expectEqualField(expected, *actualOpt, "sfAmountCommitment");
    }

    {
        auto const& expected = zKProofValue;
        auto const actualOpt = rebuiltTx.getZKProof();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfZKProof should be present";
        expectEqualField(expected, *actualOpt, "sfZKProof");
    }

    {
        auto const& expected = loanAuditorEncryptionKeyValue;
        auto const actualOpt = rebuiltTx.getLoanAuditorEncryptionKey();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanAuditorEncryptionKey should be present";
        expectEqualField(expected, *actualOpt, "sfLoanAuditorEncryptionKey");
    }

    {
        auto const& expected = loanAuditorEncryptedPrincipalValue;
        auto const actualOpt = rebuiltTx.getLoanAuditorEncryptedPrincipal();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanAuditorEncryptedPrincipal should be present";
        expectEqualField(expected, *actualOpt, "sfLoanAuditorEncryptedPrincipal");
    }

    {
        auto const& expected = borrowerEncryptedAmountOwedValue;
        auto const actualOpt = rebuiltTx.getBorrowerEncryptedAmountOwed();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBorrowerEncryptedAmountOwed should be present";
        expectEqualField(expected, *actualOpt, "sfBorrowerEncryptedAmountOwed");
    }

    {
        auto const& expected = brokerEncryptedAmountOwedValue;
        auto const actualOpt = rebuiltTx.getBrokerEncryptedAmountOwed();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBrokerEncryptedAmountOwed should be present";
        expectEqualField(expected, *actualOpt, "sfBrokerEncryptedAmountOwed");
    }

    {
        auto const& expected = loanAuditorEncryptedAmountOwedValue;
        auto const actualOpt = rebuiltTx.getLoanAuditorEncryptedAmountOwed();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanAuditorEncryptedAmountOwed should be present";
        expectEqualField(expected, *actualOpt, "sfLoanAuditorEncryptedAmountOwed");
    }

    {
        auto const& expected = borrowerEncryptedPeriodicPaymentValue;
        auto const actualOpt = rebuiltTx.getBorrowerEncryptedPeriodicPayment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBorrowerEncryptedPeriodicPayment should be present";
        expectEqualField(expected, *actualOpt, "sfBorrowerEncryptedPeriodicPayment");
    }

    {
        auto const& expected = brokerEncryptedPeriodicPaymentValue;
        auto const actualOpt = rebuiltTx.getBrokerEncryptedPeriodicPayment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfBrokerEncryptedPeriodicPayment should be present";
        expectEqualField(expected, *actualOpt, "sfBrokerEncryptedPeriodicPayment");
    }

    {
        auto const& expected = loanAuditorEncryptedPeriodicPaymentValue;
        auto const actualOpt = rebuiltTx.getLoanAuditorEncryptedPeriodicPayment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfLoanAuditorEncryptedPeriodicPayment should be present";
        expectEqualField(expected, *actualOpt, "sfLoanAuditorEncryptedPeriodicPayment");
    }

    {
        auto const& expected = amountOwedCommitmentValue;
        auto const actualOpt = rebuiltTx.getAmountOwedCommitment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfAmountOwedCommitment should be present";
        expectEqualField(expected, *actualOpt, "sfAmountOwedCommitment");
    }

    {
        auto const& expected = periodicPaymentCommitmentValue;
        auto const actualOpt = rebuiltTx.getPeriodicPaymentCommitment();
        ASSERT_TRUE(actualOpt.has_value()) << "Optional field sfPeriodicPaymentCommitment should be present";
        expectEqualField(expected, *actualOpt, "sfPeriodicPaymentCommitment");
    }

}

// 3) Verify wrapper throws when constructed from wrong transaction type.
TEST(TransactionsLoanSetTests, WrapperThrowsOnWrongTxType)
{
    // Build a valid transaction of a different type
    auto const [pk, sk] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testWrongType"));
    auto const account = calcAccountID(pk);

    AccountSetBuilder wrongBuilder{account, 1, canonical_AMOUNT()};
    auto wrongTx = wrongBuilder.build(pk, sk);

    EXPECT_THROW(LoanSet{wrongTx.getSTTx()}, std::runtime_error);
}

// 4) Verify builder throws when constructed from wrong transaction type.
TEST(TransactionsLoanSetTests, BuilderThrowsOnWrongTxType)
{
    // Build a valid transaction of a different type
    auto const [pk, sk] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testWrongTypeBuilder"));
    auto const account = calcAccountID(pk);

    AccountSetBuilder wrongBuilder{account, 1, canonical_AMOUNT()};
    auto wrongTx = wrongBuilder.build(pk, sk);

    EXPECT_THROW(LoanSetBuilder{wrongTx.getSTTx()}, std::runtime_error);
}

// 5) Build with only required fields and verify optional fields return nullopt.
TEST(TransactionsLoanSetTests, OptionalFieldsReturnNullopt)
{
    // Generate a deterministic keypair for signing
    auto const [publicKey, secretKey] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testLoanSetNullopt"));

    // Common transaction fields
    auto const accountValue = calcAccountID(publicKey);
    std::uint32_t const sequenceValue = 3;
    auto const feeValue = canonical_AMOUNT();

    // Transaction-specific required field values
    auto const loanBrokerIDValue = canonical_UINT256();

    LoanSetBuilder builder{
        accountValue,
        loanBrokerIDValue,
        sequenceValue,
        feeValue
    };

    // Do NOT set optional fields

    auto tx = builder.build(publicKey, secretKey);

    // Verify optional fields are not present
    EXPECT_FALSE(tx.hasData());
    EXPECT_FALSE(tx.getData().has_value());
    EXPECT_FALSE(tx.hasCounterparty());
    EXPECT_FALSE(tx.getCounterparty().has_value());
    EXPECT_FALSE(tx.hasCounterpartySignature());
    EXPECT_FALSE(tx.getCounterpartySignature().has_value());
    EXPECT_FALSE(tx.hasLoanOriginationFee());
    EXPECT_FALSE(tx.getLoanOriginationFee().has_value());
    EXPECT_FALSE(tx.hasLoanServiceFee());
    EXPECT_FALSE(tx.getLoanServiceFee().has_value());
    EXPECT_FALSE(tx.hasLatePaymentFee());
    EXPECT_FALSE(tx.getLatePaymentFee().has_value());
    EXPECT_FALSE(tx.hasClosePaymentFee());
    EXPECT_FALSE(tx.getClosePaymentFee().has_value());
    EXPECT_FALSE(tx.hasOverpaymentFee());
    EXPECT_FALSE(tx.getOverpaymentFee().has_value());
    EXPECT_FALSE(tx.hasInterestRate());
    EXPECT_FALSE(tx.getInterestRate().has_value());
    EXPECT_FALSE(tx.hasLateInterestRate());
    EXPECT_FALSE(tx.getLateInterestRate().has_value());
    EXPECT_FALSE(tx.hasCloseInterestRate());
    EXPECT_FALSE(tx.getCloseInterestRate().has_value());
    EXPECT_FALSE(tx.hasOverpaymentInterestRate());
    EXPECT_FALSE(tx.getOverpaymentInterestRate().has_value());
    EXPECT_FALSE(tx.hasPrincipalRequested());
    EXPECT_FALSE(tx.getPrincipalRequested().has_value());
    EXPECT_FALSE(tx.hasPaymentTotal());
    EXPECT_FALSE(tx.getPaymentTotal().has_value());
    EXPECT_FALSE(tx.hasPaymentInterval());
    EXPECT_FALSE(tx.getPaymentInterval().has_value());
    EXPECT_FALSE(tx.hasGracePeriod());
    EXPECT_FALSE(tx.getGracePeriod().has_value());
    EXPECT_FALSE(tx.hasSenderEncryptedAmount());
    EXPECT_FALSE(tx.getSenderEncryptedAmount().has_value());
    EXPECT_FALSE(tx.hasDestinationEncryptedAmount());
    EXPECT_FALSE(tx.getDestinationEncryptedAmount().has_value());
    EXPECT_FALSE(tx.hasIssuerEncryptedAmount());
    EXPECT_FALSE(tx.getIssuerEncryptedAmount().has_value());
    EXPECT_FALSE(tx.hasAuditorEncryptedAmount());
    EXPECT_FALSE(tx.getAuditorEncryptedAmount().has_value());
    EXPECT_FALSE(tx.hasBalanceCommitment());
    EXPECT_FALSE(tx.getBalanceCommitment().has_value());
    EXPECT_FALSE(tx.hasAmountCommitment());
    EXPECT_FALSE(tx.getAmountCommitment().has_value());
    EXPECT_FALSE(tx.hasZKProof());
    EXPECT_FALSE(tx.getZKProof().has_value());
    EXPECT_FALSE(tx.hasLoanAuditorEncryptionKey());
    EXPECT_FALSE(tx.getLoanAuditorEncryptionKey().has_value());
    EXPECT_FALSE(tx.hasLoanAuditorEncryptedPrincipal());
    EXPECT_FALSE(tx.getLoanAuditorEncryptedPrincipal().has_value());
    EXPECT_FALSE(tx.hasBorrowerEncryptedAmountOwed());
    EXPECT_FALSE(tx.getBorrowerEncryptedAmountOwed().has_value());
    EXPECT_FALSE(tx.hasBrokerEncryptedAmountOwed());
    EXPECT_FALSE(tx.getBrokerEncryptedAmountOwed().has_value());
    EXPECT_FALSE(tx.hasLoanAuditorEncryptedAmountOwed());
    EXPECT_FALSE(tx.getLoanAuditorEncryptedAmountOwed().has_value());
    EXPECT_FALSE(tx.hasBorrowerEncryptedPeriodicPayment());
    EXPECT_FALSE(tx.getBorrowerEncryptedPeriodicPayment().has_value());
    EXPECT_FALSE(tx.hasBrokerEncryptedPeriodicPayment());
    EXPECT_FALSE(tx.getBrokerEncryptedPeriodicPayment().has_value());
    EXPECT_FALSE(tx.hasLoanAuditorEncryptedPeriodicPayment());
    EXPECT_FALSE(tx.getLoanAuditorEncryptedPeriodicPayment().has_value());
    EXPECT_FALSE(tx.hasAmountOwedCommitment());
    EXPECT_FALSE(tx.getAmountOwedCommitment().has_value());
    EXPECT_FALSE(tx.hasPeriodicPaymentCommitment());
    EXPECT_FALSE(tx.getPeriodicPaymentCommitment().has_value());
}

}
