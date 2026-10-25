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

class LoanBrokerSettleBuilder;

/**
 * @brief Transaction: LoanBrokerSettle
 *
 * Type: ttLOAN_BROKER_SETTLE (83)
 * Delegable: Delegation::NotDelegable
 * Amendment: featureConfidentialLendingProtocol
 * Privileges: Privilege::MustModifyVault
 *
 * Immutable wrapper around STTx providing type-safe field access.
 * Use LoanBrokerSettleBuilder to construct new transactions.
 */
class LoanBrokerSettle : public TransactionBase
{
public:
    static constexpr xrpl::TxType txType = ttLOAN_BROKER_SETTLE;

    /**
     * @brief Construct a LoanBrokerSettle transaction wrapper from an existing STTx object.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    explicit LoanBrokerSettle(std::shared_ptr<STTx const> tx)
        : TransactionBase(std::move(tx))
    {
        // Verify transaction type
        if (tx_->getTxnType() != txType)
        {
            throw std::runtime_error("Invalid transaction type for LoanBrokerSettle");
        }
    }

    // Transaction-specific field getters

    /**
     * @brief Get sfLoanBrokerID (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_UINT256::type::value_type
    getLoanBrokerID() const
    {
        return this->tx_->at(sfLoanBrokerID);
    }

    /**
     * @brief Get sfAmount (SoeRequired)
     * @note This field supports MPT (Multi-Purpose Token) amounts.
     * @return The field value.
     */
    [[nodiscard]]
    SF_AMOUNT::type::value_type
    getAmount() const
    {
        return this->tx_->at(sfAmount);
    }

    /**
     * @brief Get sfHolderEncryptedAmount (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_VL::type::value_type
    getHolderEncryptedAmount() const
    {
        return this->tx_->at(sfHolderEncryptedAmount);
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
     * @brief Get sfBlindingFactor (SoeRequired)
     * @return The field value.
     */
    [[nodiscard]]
    SF_UINT256::type::value_type
    getBlindingFactor() const
    {
        return this->tx_->at(sfBlindingFactor);
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
 * @brief Builder for LoanBrokerSettle transactions.
 *
 * Provides a fluent interface for constructing transactions with method chaining.
 * Uses STObject internally for flexible transaction construction.
 * Inherits common field setters from TransactionBuilderBase.
 */
class LoanBrokerSettleBuilder : public TransactionBuilderBase<LoanBrokerSettleBuilder>
{
public:
    /**
     * @brief Construct a new LoanBrokerSettleBuilder with required fields.
     * @param account The account initiating the transaction.
     * @param loanBrokerID The sfLoanBrokerID field value.
     * @param amount The sfAmount field value.
     * @param holderEncryptedAmount The sfHolderEncryptedAmount field value.
     * @param issuerEncryptedAmount The sfIssuerEncryptedAmount field value.
     * @param auditorEncryptedAmount The sfAuditorEncryptedAmount field value.
     * @param blindingFactor The sfBlindingFactor field value.
     * @param balanceCommitment The sfBalanceCommitment field value.
     * @param zKProof The sfZKProof field value.
     * @param sequence Optional sequence number for the transaction.
     * @param fee Optional fee for the transaction.
     */
    LoanBrokerSettleBuilder(SF_ACCOUNT::type::value_type account,
                     std::decay_t<typename SF_UINT256::type::value_type> const& loanBrokerID,                     std::decay_t<typename SF_AMOUNT::type::value_type> const& amount,                     std::decay_t<typename SF_VL::type::value_type> const& holderEncryptedAmount,                     std::decay_t<typename SF_VL::type::value_type> const& issuerEncryptedAmount,                     std::decay_t<typename SF_VL::type::value_type> const& auditorEncryptedAmount,                     std::decay_t<typename SF_UINT256::type::value_type> const& blindingFactor,                     std::decay_t<typename SF_VL::type::value_type> const& balanceCommitment,                     std::decay_t<typename SF_VL::type::value_type> const& zKProof,                    std::optional<SF_UINT32::type::value_type> sequence = std::nullopt,
                    std::optional<SF_AMOUNT::type::value_type> fee = std::nullopt
)
        : TransactionBuilderBase<LoanBrokerSettleBuilder>(ttLOAN_BROKER_SETTLE, account, sequence, fee)
    {
        setLoanBrokerID(loanBrokerID);
        setAmount(amount);
        setHolderEncryptedAmount(holderEncryptedAmount);
        setIssuerEncryptedAmount(issuerEncryptedAmount);
        setAuditorEncryptedAmount(auditorEncryptedAmount);
        setBlindingFactor(blindingFactor);
        setBalanceCommitment(balanceCommitment);
        setZKProof(zKProof);
    }

    /**
     * @brief Construct a LoanBrokerSettleBuilder from an existing STTx object.
     * @param tx The existing transaction to copy from.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    LoanBrokerSettleBuilder(std::shared_ptr<STTx const> tx)
    {
        if (tx->getTxnType() != ttLOAN_BROKER_SETTLE)
        {
            throw std::runtime_error("Invalid transaction type for LoanBrokerSettleBuilder");
        }
        object_ = *tx;
    }

    /**
     * @brief Transaction-specific field setters
     */

    /**
     * @brief Set sfLoanBrokerID (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerSettleBuilder&
    setLoanBrokerID(std::decay_t<typename SF_UINT256::type::value_type> const& value)
    {
        object_[sfLoanBrokerID] = value;
        return *this;
    }

    /**
     * @brief Set sfAmount (SoeRequired)
     * @note This field supports MPT (Multi-Purpose Token) amounts.
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerSettleBuilder&
    setAmount(std::decay_t<typename SF_AMOUNT::type::value_type> const& value)
    {
        object_[sfAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfHolderEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerSettleBuilder&
    setHolderEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfHolderEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfIssuerEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerSettleBuilder&
    setIssuerEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfIssuerEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfAuditorEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerSettleBuilder&
    setAuditorEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfAuditorEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfBlindingFactor (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerSettleBuilder&
    setBlindingFactor(std::decay_t<typename SF_UINT256::type::value_type> const& value)
    {
        object_[sfBlindingFactor] = value;
        return *this;
    }

    /**
     * @brief Set sfBalanceCommitment (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerSettleBuilder&
    setBalanceCommitment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfBalanceCommitment] = value;
        return *this;
    }

    /**
     * @brief Set sfZKProof (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerSettleBuilder&
    setZKProof(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfZKProof] = value;
        return *this;
    }

    /**
     * @brief Build and return the LoanBrokerSettle wrapper.
     * @param publicKey The public key for signing.
     * @param secretKey The secret key for signing.
     * @return The constructed transaction wrapper.
     */
    LoanBrokerSettle
    build(PublicKey const& publicKey, SecretKey const& secretKey)
    {
        sign(publicKey, secretKey);
        return LoanBrokerSettle{std::make_shared<STTx>(std::move(object_))};
    }
};

}  // namespace xrpl::transactions
