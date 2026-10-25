// Auto-generated unit tests for transaction ConfidentialLoanPay


#include <gtest/gtest.h>

#include <protocol_autogen/TestHelpers.h>

#include <xrpl/protocol/SecretKey.h>
#include <xrpl/protocol/Seed.h>
#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol_autogen/transactions/ConfidentialLoanPay.h>
#include <xrpl/protocol_autogen/transactions/AccountSet.h>

#include <string>

namespace xrpl::transactions {

// 1 & 4) Set fields via builder setters, build, then read them back via
// wrapper getters. After build(), validate() should succeed.
TEST(TransactionsConfidentialLoanPayTests, BuilderSettersRoundTrip)
{
    // Generate a deterministic keypair for signing
    auto const [publicKey, secretKey] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testConfidentialLoanPay"));

    // Common transaction fields
    auto const accountValue = calcAccountID(publicKey);
    std::uint32_t const sequenceValue = 1;
    auto const feeValue = canonical_AMOUNT();

    // Transaction-specific field values
    auto const loanIDValue = canonical_UINT256();
    auto const senderEncryptedAmountValue = canonical_VL();
    auto const destinationEncryptedAmountValue = canonical_VL();
    auto const issuerEncryptedAmountValue = canonical_VL();
    auto const auditorEncryptedAmountValue = canonical_VL();
    auto const loanAuditorEncryptedAmountValue = canonical_VL();
    auto const balanceCommitmentValue = canonical_VL();
    auto const amountCommitmentValue = canonical_VL();
    auto const amountOwedCommitmentValue = canonical_VL();
    auto const periodicPaymentCommitmentValue = canonical_VL();
    auto const zKProofValue = canonical_VL();

    ConfidentialLoanPayBuilder builder{
        accountValue,
        loanIDValue,
        senderEncryptedAmountValue,
        destinationEncryptedAmountValue,
        issuerEncryptedAmountValue,
        auditorEncryptedAmountValue,
        loanAuditorEncryptedAmountValue,
        balanceCommitmentValue,
        amountCommitmentValue,
        zKProofValue,
        sequenceValue,
        feeValue
    };

    // Set optional fields
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
        auto const& expected = loanIDValue;
        auto const actual = tx.getLoanID();
        expectEqualField(expected, actual, "sfLoanID");
    }

    {
        auto const& expected = senderEncryptedAmountValue;
        auto const actual = tx.getSenderEncryptedAmount();
        expectEqualField(expected, actual, "sfSenderEncryptedAmount");
    }

    {
        auto const& expected = destinationEncryptedAmountValue;
        auto const actual = tx.getDestinationEncryptedAmount();
        expectEqualField(expected, actual, "sfDestinationEncryptedAmount");
    }

    {
        auto const& expected = issuerEncryptedAmountValue;
        auto const actual = tx.getIssuerEncryptedAmount();
        expectEqualField(expected, actual, "sfIssuerEncryptedAmount");
    }

    {
        auto const& expected = auditorEncryptedAmountValue;
        auto const actual = tx.getAuditorEncryptedAmount();
        expectEqualField(expected, actual, "sfAuditorEncryptedAmount");
    }

    {
        auto const& expected = loanAuditorEncryptedAmountValue;
        auto const actual = tx.getLoanAuditorEncryptedAmount();
        expectEqualField(expected, actual, "sfLoanAuditorEncryptedAmount");
    }

    {
        auto const& expected = balanceCommitmentValue;
        auto const actual = tx.getBalanceCommitment();
        expectEqualField(expected, actual, "sfBalanceCommitment");
    }

    {
        auto const& expected = amountCommitmentValue;
        auto const actual = tx.getAmountCommitment();
        expectEqualField(expected, actual, "sfAmountCommitment");
    }

    {
        auto const& expected = zKProofValue;
        auto const actual = tx.getZKProof();
        expectEqualField(expected, actual, "sfZKProof");
    }

    // Verify optional fields
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
TEST(TransactionsConfidentialLoanPayTests, BuilderFromStTxRoundTrip)
{
    // Generate a deterministic keypair for signing
    auto const [publicKey, secretKey] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testConfidentialLoanPayFromTx"));

    // Common transaction fields
    auto const accountValue = calcAccountID(publicKey);
    std::uint32_t const sequenceValue = 2;
    auto const feeValue = canonical_AMOUNT();

    // Transaction-specific field values
    auto const loanIDValue = canonical_UINT256();
    auto const senderEncryptedAmountValue = canonical_VL();
    auto const destinationEncryptedAmountValue = canonical_VL();
    auto const issuerEncryptedAmountValue = canonical_VL();
    auto const auditorEncryptedAmountValue = canonical_VL();
    auto const loanAuditorEncryptedAmountValue = canonical_VL();
    auto const balanceCommitmentValue = canonical_VL();
    auto const amountCommitmentValue = canonical_VL();
    auto const amountOwedCommitmentValue = canonical_VL();
    auto const periodicPaymentCommitmentValue = canonical_VL();
    auto const zKProofValue = canonical_VL();

    // Build an initial transaction
    ConfidentialLoanPayBuilder initialBuilder{
        accountValue,
        loanIDValue,
        senderEncryptedAmountValue,
        destinationEncryptedAmountValue,
        issuerEncryptedAmountValue,
        auditorEncryptedAmountValue,
        loanAuditorEncryptedAmountValue,
        balanceCommitmentValue,
        amountCommitmentValue,
        zKProofValue,
        sequenceValue,
        feeValue
    };

    initialBuilder.setAmountOwedCommitment(amountOwedCommitmentValue);
    initialBuilder.setPeriodicPaymentCommitment(periodicPaymentCommitmentValue);

    auto initialTx = initialBuilder.build(publicKey, secretKey);

    // Create builder from existing STTx
    ConfidentialLoanPayBuilder builderFromTx{initialTx.getSTTx()};

    auto rebuiltTx = builderFromTx.build(publicKey, secretKey);

    std::string reason;
    EXPECT_TRUE(rebuiltTx.validate(reason)) << reason;

    // Verify common fields
    EXPECT_EQ(rebuiltTx.getAccount(), accountValue);
    EXPECT_EQ(rebuiltTx.getSequence(), sequenceValue);
    EXPECT_EQ(rebuiltTx.getFee(), feeValue);

    // Verify required fields
    {
        auto const& expected = loanIDValue;
        auto const actual = rebuiltTx.getLoanID();
        expectEqualField(expected, actual, "sfLoanID");
    }

    {
        auto const& expected = senderEncryptedAmountValue;
        auto const actual = rebuiltTx.getSenderEncryptedAmount();
        expectEqualField(expected, actual, "sfSenderEncryptedAmount");
    }

    {
        auto const& expected = destinationEncryptedAmountValue;
        auto const actual = rebuiltTx.getDestinationEncryptedAmount();
        expectEqualField(expected, actual, "sfDestinationEncryptedAmount");
    }

    {
        auto const& expected = issuerEncryptedAmountValue;
        auto const actual = rebuiltTx.getIssuerEncryptedAmount();
        expectEqualField(expected, actual, "sfIssuerEncryptedAmount");
    }

    {
        auto const& expected = auditorEncryptedAmountValue;
        auto const actual = rebuiltTx.getAuditorEncryptedAmount();
        expectEqualField(expected, actual, "sfAuditorEncryptedAmount");
    }

    {
        auto const& expected = loanAuditorEncryptedAmountValue;
        auto const actual = rebuiltTx.getLoanAuditorEncryptedAmount();
        expectEqualField(expected, actual, "sfLoanAuditorEncryptedAmount");
    }

    {
        auto const& expected = balanceCommitmentValue;
        auto const actual = rebuiltTx.getBalanceCommitment();
        expectEqualField(expected, actual, "sfBalanceCommitment");
    }

    {
        auto const& expected = amountCommitmentValue;
        auto const actual = rebuiltTx.getAmountCommitment();
        expectEqualField(expected, actual, "sfAmountCommitment");
    }

    {
        auto const& expected = zKProofValue;
        auto const actual = rebuiltTx.getZKProof();
        expectEqualField(expected, actual, "sfZKProof");
    }

    // Verify optional fields
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
TEST(TransactionsConfidentialLoanPayTests, WrapperThrowsOnWrongTxType)
{
    // Build a valid transaction of a different type
    auto const [pk, sk] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testWrongType"));
    auto const account = calcAccountID(pk);

    AccountSetBuilder wrongBuilder{account, 1, canonical_AMOUNT()};
    auto wrongTx = wrongBuilder.build(pk, sk);

    EXPECT_THROW(ConfidentialLoanPay{wrongTx.getSTTx()}, std::runtime_error);
}

// 4) Verify builder throws when constructed from wrong transaction type.
TEST(TransactionsConfidentialLoanPayTests, BuilderThrowsOnWrongTxType)
{
    // Build a valid transaction of a different type
    auto const [pk, sk] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testWrongTypeBuilder"));
    auto const account = calcAccountID(pk);

    AccountSetBuilder wrongBuilder{account, 1, canonical_AMOUNT()};
    auto wrongTx = wrongBuilder.build(pk, sk);

    EXPECT_THROW(ConfidentialLoanPayBuilder{wrongTx.getSTTx()}, std::runtime_error);
}

// 5) Build with only required fields and verify optional fields return nullopt.
TEST(TransactionsConfidentialLoanPayTests, OptionalFieldsReturnNullopt)
{
    // Generate a deterministic keypair for signing
    auto const [publicKey, secretKey] =
        generateKeyPair(KeyType::Secp256k1, generateSeed("testConfidentialLoanPayNullopt"));

    // Common transaction fields
    auto const accountValue = calcAccountID(publicKey);
    std::uint32_t const sequenceValue = 3;
    auto const feeValue = canonical_AMOUNT();

    // Transaction-specific required field values
    auto const loanIDValue = canonical_UINT256();
    auto const senderEncryptedAmountValue = canonical_VL();
    auto const destinationEncryptedAmountValue = canonical_VL();
    auto const issuerEncryptedAmountValue = canonical_VL();
    auto const auditorEncryptedAmountValue = canonical_VL();
    auto const loanAuditorEncryptedAmountValue = canonical_VL();
    auto const balanceCommitmentValue = canonical_VL();
    auto const amountCommitmentValue = canonical_VL();
    auto const zKProofValue = canonical_VL();

    ConfidentialLoanPayBuilder builder{
        accountValue,
        loanIDValue,
        senderEncryptedAmountValue,
        destinationEncryptedAmountValue,
        issuerEncryptedAmountValue,
        auditorEncryptedAmountValue,
        loanAuditorEncryptedAmountValue,
        balanceCommitmentValue,
        amountCommitmentValue,
        zKProofValue,
        sequenceValue,
        feeValue
    };

    // Do NOT set optional fields

    auto tx = builder.build(publicKey, secretKey);

    // Verify optional fields are not present
    EXPECT_FALSE(tx.hasAmountOwedCommitment());
    EXPECT_FALSE(tx.getAmountOwedCommitment().has_value());
    EXPECT_FALSE(tx.hasPeriodicPaymentCommitment());
    EXPECT_FALSE(tx.getPeriodicPaymentCommitment().has_value());
}

}
