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

class LoanBrokerDrawBuilder;

/**
 * @brief Transaction: LoanBrokerDraw
 *
 * Type: ttLOAN_BROKER_DRAW (97)
 * Delegable: Delegation::NotDelegable
 * Amendment: featureConfidentialLendingProtocol
 * Privileges: Privilege::MustModifyVault
 *
 * Immutable wrapper around STTx providing type-safe field access.
 * Use LoanBrokerDrawBuilder to construct new transactions.
 */
class LoanBrokerDraw : public TransactionBase
{
public:
    static constexpr xrpl::TxType txType = ttLOAN_BROKER_DRAW;

    /**
     * @brief Construct a LoanBrokerDraw transaction wrapper from an existing STTx object.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    explicit LoanBrokerDraw(std::shared_ptr<STTx const> tx)
        : TransactionBase(std::move(tx))
    {
        // Verify transaction type
        if (tx_->getTxnType() != txType)
        {
            throw std::runtime_error("Invalid transaction type for LoanBrokerDraw");
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
};

/**
 * @brief Builder for LoanBrokerDraw transactions.
 *
 * Provides a fluent interface for constructing transactions with method chaining.
 * Uses STObject internally for flexible transaction construction.
 * Inherits common field setters from TransactionBuilderBase.
 */
class LoanBrokerDrawBuilder : public TransactionBuilderBase<LoanBrokerDrawBuilder>
{
public:
    /**
     * @brief Construct a new LoanBrokerDrawBuilder with required fields.
     * @param account The account initiating the transaction.
     * @param loanBrokerID The sfLoanBrokerID field value.
     * @param amount The sfAmount field value.
     * @param holderEncryptedAmount The sfHolderEncryptedAmount field value.
     * @param issuerEncryptedAmount The sfIssuerEncryptedAmount field value.
     * @param auditorEncryptedAmount The sfAuditorEncryptedAmount field value.
     * @param blindingFactor The sfBlindingFactor field value.
     * @param sequence Optional sequence number for the transaction.
     * @param fee Optional fee for the transaction.
     */
    LoanBrokerDrawBuilder(SF_ACCOUNT::type::value_type account,
                     std::decay_t<typename SF_UINT256::type::value_type> const& loanBrokerID,                     std::decay_t<typename SF_AMOUNT::type::value_type> const& amount,                     std::decay_t<typename SF_VL::type::value_type> const& holderEncryptedAmount,                     std::decay_t<typename SF_VL::type::value_type> const& issuerEncryptedAmount,                     std::decay_t<typename SF_VL::type::value_type> const& auditorEncryptedAmount,                     std::decay_t<typename SF_UINT256::type::value_type> const& blindingFactor,                    std::optional<SF_UINT32::type::value_type> sequence = std::nullopt,
                    std::optional<SF_AMOUNT::type::value_type> fee = std::nullopt
)
        : TransactionBuilderBase<LoanBrokerDrawBuilder>(ttLOAN_BROKER_DRAW, account, sequence, fee)
    {
        setLoanBrokerID(loanBrokerID);
        setAmount(amount);
        setHolderEncryptedAmount(holderEncryptedAmount);
        setIssuerEncryptedAmount(issuerEncryptedAmount);
        setAuditorEncryptedAmount(auditorEncryptedAmount);
        setBlindingFactor(blindingFactor);
    }

    /**
     * @brief Construct a LoanBrokerDrawBuilder from an existing STTx object.
     * @param tx The existing transaction to copy from.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    LoanBrokerDrawBuilder(std::shared_ptr<STTx const> tx)
    {
        if (tx->getTxnType() != ttLOAN_BROKER_DRAW)
        {
            throw std::runtime_error("Invalid transaction type for LoanBrokerDrawBuilder");
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
    LoanBrokerDrawBuilder&
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
    LoanBrokerDrawBuilder&
    setAmount(std::decay_t<typename SF_AMOUNT::type::value_type> const& value)
    {
        object_[sfAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfHolderEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerDrawBuilder&
    setHolderEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfHolderEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfIssuerEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerDrawBuilder&
    setIssuerEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfIssuerEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfAuditorEncryptedAmount (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerDrawBuilder&
    setAuditorEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfAuditorEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfBlindingFactor (SoeRequired)
     * @return Reference to this builder for method chaining.
     */
    LoanBrokerDrawBuilder&
    setBlindingFactor(std::decay_t<typename SF_UINT256::type::value_type> const& value)
    {
        object_[sfBlindingFactor] = value;
        return *this;
    }

    /**
     * @brief Build and return the LoanBrokerDraw wrapper.
     * @param publicKey The public key for signing.
     * @param secretKey The secret key for signing.
     * @return The constructed transaction wrapper.
     */
    LoanBrokerDraw
    build(PublicKey const& publicKey, SecretKey const& secretKey)
    {
        sign(publicKey, secretKey);
        return LoanBrokerDraw{std::make_shared<STTx>(std::move(object_))};
    }
};

}  // namespace xrpl::transactions
