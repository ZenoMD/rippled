// This file is auto-generated. Do not edit.
#pragma once

#include <xrpl/protocol/STTx.h>
#include <xrpl/protocol/STParsedJSON.h>
#include <xrpl/protocol/jss.h>
#include <xrpl/protocol_autogen/TransactionBase.h>
#include <xrpl/protocol_autogen/TransactionBuilderBase.h>
#include <xrpl/json/json_value.h>

#include <stdexcept>
#include <optional>

namespace xrpl::transactions {

class ConfidentialLoanPayBuilder;

/**
 * @brief Transaction: ConfidentialLoanPay
 *
 * Type: ttCONFIDENTIAL_LOAN_PAY (99)
 * Delegable: Delegation::NotDelegable
 * Amendment: featureConfidentialLendingProtocol
 * Privileges: Privilege::NoPriv
 *
 * Immutable wrapper around STTx providing type-safe field access.
 * Use ConfidentialLoanPayBuilder to construct new transactions.
 */
class ConfidentialLoanPay : public TransactionBase
{
public:
    static constexpr xrpl::TxType txType = ttCONFIDENTIAL_LOAN_PAY;

    /**
     * @brief Construct a ConfidentialLoanPay transaction wrapper from an existing STTx object.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    explicit ConfidentialLoanPay(std::shared_ptr<STTx const> tx)
        : TransactionBase(std::move(tx))
    {
        // Verify transaction type
        if (tx_->getTxnType() != txType)
        {
            throw std::runtime_error("Invalid transaction type for ConfidentialLoanPay");
        }
    }

    // Transaction-specific field getters

    /**
     * @brief Get sfLoanID (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_UINT256::type::value_type
    getLoanID() const
    {
        return this->tx_->at(sfLoanID);
    }

    /**
     * @brief Get sfSenderEncryptedAmount (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_VL::type::value_type
    getSenderEncryptedAmount() const
    {
        return this->tx_->at(sfSenderEncryptedAmount);
    }

    /**
     * @brief Get sfDestinationEncryptedAmount (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_VL::type::value_type
    getDestinationEncryptedAmount() const
    {
        return this->tx_->at(sfDestinationEncryptedAmount);
    }

    /**
     * @brief Get sfIssuerEncryptedAmount (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_VL::type::value_type
    getIssuerEncryptedAmount() const
    {
        return this->tx_->at(sfIssuerEncryptedAmount);
    }

    /**
     * @brief Get sfAuditorEncryptedAmount (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_VL::type::value_type
    getAuditorEncryptedAmount() const
    {
        return this->tx_->at(sfAuditorEncryptedAmount);
    }

    /**
     * @brief Get sfLoanAuditorEncryptedAmount (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_VL::type::value_type
    getLoanAuditorEncryptedAmount() const
    {
        return this->tx_->at(sfLoanAuditorEncryptedAmount);
    }

    /**
     * @brief Get sfBalanceCommitment (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_VL::type::value_type
    getBalanceCommitment() const
    {
        return this->tx_->at(sfBalanceCommitment);
    }

    /**
     * @brief Get sfAmountCommitment (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_VL::type::value_type
    getAmountCommitment() const
    {
        return this->tx_->at(sfAmountCommitment);
    }

    /**
     * @brief Get sfAmountOwedCommitment (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getAmountOwedCommitment() const
    {
        if (hasAmountOwedCommitment())
        {
            return this->tx_->at(sfAmountOwedCommitment);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfAmountOwedCommitment is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasAmountOwedCommitment() const
    {
        return this->tx_->isFieldPresent(sfAmountOwedCommitment);
    }

    /**
     * @brief Get sfPeriodicPaymentCommitment (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getPeriodicPaymentCommitment() const
    {
        if (hasPeriodicPaymentCommitment())
        {
            return this->tx_->at(sfPeriodicPaymentCommitment);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfPeriodicPaymentCommitment is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasPeriodicPaymentCommitment() const
    {
        return this->tx_->isFieldPresent(sfPeriodicPaymentCommitment);
    }

    /**
     * @brief Get sfZKProof (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_VL::type::value_type
    getZKProof() const
    {
        return this->tx_->at(sfZKProof);
    }
};

/**
 * @brief Builder for ConfidentialLoanPay transactions.
 *
 * Provides a fluent interface for constructing transactions with method chaining.
 * Uses STObject internally for flexible transaction construction.
 * Inherits common field setters from TransactionBuilderBase.
 */
class ConfidentialLoanPayBuilder : public TransactionBuilderBase<ConfidentialLoanPayBuilder>
{
public:
    /**
     * @brief Construct a new ConfidentialLoanPayBuilder with required fields.
     * @param account The account initiating the transaction.
     * @param loanID The sfLoanID field value.
     * @param senderEncryptedAmount The sfSenderEncryptedAmount field value.
     * @param destinationEncryptedAmount The sfDestinationEncryptedAmount field value.
     * @param issuerEncryptedAmount The sfIssuerEncryptedAmount field value.
     * @param auditorEncryptedAmount The sfAuditorEncryptedAmount field value.
     * @param loanAuditorEncryptedAmount The sfLoanAuditorEncryptedAmount field value.
     * @param balanceCommitment The sfBalanceCommitment field value.
     * @param amountCommitment The sfAmountCommitment field value.
     * @param zKProof The sfZKProof field value.
     * @param sequence Optional sequence number for the transaction.
     * @param fee Optional fee for the transaction.
     */
    ConfidentialLoanPayBuilder(SF_ACCOUNT::type::value_type account,
                     std::decay_t<typename SF_UINT256::type::value_type> const& loanID,                     std::decay_t<typename SF_VL::type::value_type> const& senderEncryptedAmount,                     std::decay_t<typename SF_VL::type::value_type> const& destinationEncryptedAmount,                     std::decay_t<typename SF_VL::type::value_type> const& issuerEncryptedAmount,                     std::decay_t<typename SF_VL::type::value_type> const& auditorEncryptedAmount,                     std::decay_t<typename SF_VL::type::value_type> const& loanAuditorEncryptedAmount,                     std::decay_t<typename SF_VL::type::value_type> const& balanceCommitment,                     std::decay_t<typename SF_VL::type::value_type> const& amountCommitment,                     std::decay_t<typename SF_VL::type::value_type> const& zKProof,                    std::optional<SF_UINT32::type::value_type> sequence = std::nullopt,
                    std::optional<SF_AMOUNT::type::value_type> fee = std::nullopt
)
        : TransactionBuilderBase<ConfidentialLoanPayBuilder>(ttCONFIDENTIAL_LOAN_PAY, account, sequence, fee)
    {
        setLoanID(loanID);
        setSenderEncryptedAmount(senderEncryptedAmount);
        setDestinationEncryptedAmount(destinationEncryptedAmount);
        setIssuerEncryptedAmount(issuerEncryptedAmount);
        setAuditorEncryptedAmount(auditorEncryptedAmount);
        setLoanAuditorEncryptedAmount(loanAuditorEncryptedAmount);
        setBalanceCommitment(balanceCommitment);
        setAmountCommitment(amountCommitment);
        setZKProof(zKProof);
    }

    /**
     * @brief Construct a ConfidentialLoanPayBuilder from an existing STTx object.
     * @param tx The existing transaction to copy from.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    ConfidentialLoanPayBuilder(std::shared_ptr<STTx const> tx)
    {
        if (tx->getTxnType() != ttCONFIDENTIAL_LOAN_PAY)
        {
            throw std::runtime_error("Invalid transaction type for ConfidentialLoanPayBuilder");
        }
        object_ = *tx;
    }

    /**
     * @brief Transaction-specific field setters
     */

    /**
     * @brief Set sfLoanID (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setLoanID(std::decay_t<typename SF_UINT256::type::value_type> const& value)
    {
        object_[sfLoanID] = value;
        return *this;
    }

    /**
     * @brief Set sfSenderEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setSenderEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfSenderEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfDestinationEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setDestinationEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfDestinationEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfIssuerEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setIssuerEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfIssuerEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfAuditorEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setAuditorEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfAuditorEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfLoanAuditorEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setLoanAuditorEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfLoanAuditorEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfBalanceCommitment (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setBalanceCommitment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfBalanceCommitment] = value;
        return *this;
    }

    /**
     * @brief Set sfAmountCommitment (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setAmountCommitment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfAmountCommitment] = value;
        return *this;
    }

    /**
     * @brief Set sfAmountOwedCommitment (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setAmountOwedCommitment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfAmountOwedCommitment] = value;
        return *this;
    }

    /**
     * @brief Set sfPeriodicPaymentCommitment (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setPeriodicPaymentCommitment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfPeriodicPaymentCommitment] = value;
        return *this;
    }

    /**
     * @brief Set sfZKProof (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    ConfidentialLoanPayBuilder&
    setZKProof(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfZKProof] = value;
        return *this;
    }

    /**
     * @brief Build and return the ConfidentialLoanPay wrapper.
     * @param publicKey The public key for signing.
     * @param secretKey The secret key for signing.
     * @return The constructed transaction wrapper.
     */
    ConfidentialLoanPay
    build(PublicKey const& publicKey, SecretKey const& secretKey)
    {
        sign(publicKey, secretKey);
        return ConfidentialLoanPay{std::make_shared<STTx>(std::move(object_))};
    }
};

}  // namespace xrpl::transactions
