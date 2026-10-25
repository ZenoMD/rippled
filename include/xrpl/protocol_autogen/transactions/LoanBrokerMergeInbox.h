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

class LoanBrokerMergeInboxBuilder;

/**
 * @brief Transaction: LoanBrokerMergeInbox
 *
 * Type: ttLOAN_BROKER_MERGE_INBOX (98)
 * Delegable: Delegation::NotDelegable
 * Amendment: featureConfidentialLendingProtocol
 * Privileges: Privilege::NoPriv
 *
 * Immutable wrapper around STTx providing type-safe field access.
 * Use LoanBrokerMergeInboxBuilder to construct new transactions.
 */
class LoanBrokerMergeInbox : public TransactionBase
{
public:
    static constexpr xrpl::TxType txType = ttLOAN_BROKER_MERGE_INBOX;

    /**
     * @brief Construct a LoanBrokerMergeInbox transaction wrapper from an existing STTx object.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    explicit LoanBrokerMergeInbox(std::shared_ptr<STTx const> tx)
        : TransactionBase(std::move(tx))
    {
        // Verify transaction type
        if (tx_->getTxnType() != txType)
        {
            throw std::runtime_error("Invalid transaction type for LoanBrokerMergeInbox");
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
};

/**
 * @brief Builder for LoanBrokerMergeInbox transactions.
 *
 * Provides a fluent interface for constructing transactions with method chaining.
 * Uses STObject internally for flexible transaction construction.
 * Inherits common field setters from TransactionBuilderBase.
 */
class LoanBrokerMergeInboxBuilder : public TransactionBuilderBase<LoanBrokerMergeInboxBuilder>
{
public:
    /**
     * @brief Construct a new LoanBrokerMergeInboxBuilder with required fields.
     * @param account The account initiating the transaction.
     * @param loanBrokerID The sfLoanBrokerID field value.
     * @param sequence Optional sequence number for the transaction.
     * @param fee Optional fee for the transaction.
     */
    LoanBrokerMergeInboxBuilder(SF_ACCOUNT::type::value_type account,
                     std::decay_t<typename SF_UINT256::type::value_type> const& loanBrokerID,                    std::optional<SF_UINT32::type::value_type> sequence = std::nullopt,
                    std::optional<SF_AMOUNT::type::value_type> fee = std::nullopt
)
        : TransactionBuilderBase<LoanBrokerMergeInboxBuilder>(ttLOAN_BROKER_MERGE_INBOX, account, sequence, fee)
    {
        setLoanBrokerID(loanBrokerID);
    }

    /**
     * @brief Construct a LoanBrokerMergeInboxBuilder from an existing STTx object.
     * @param tx The existing transaction to copy from.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    LoanBrokerMergeInboxBuilder(std::shared_ptr<STTx const> tx)
    {
        if (tx->getTxnType() != ttLOAN_BROKER_MERGE_INBOX)
        {
            throw std::runtime_error("Invalid transaction type for LoanBrokerMergeInboxBuilder");
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
    LoanBrokerMergeInboxBuilder&
    setLoanBrokerID(std::decay_t<typename SF_UINT256::type::value_type> const& value)
    {
        object_[sfLoanBrokerID] = value;
        return *this;
    }

    /**
     * @brief Build and return the LoanBrokerMergeInbox wrapper.
     * @param publicKey The public key for signing.
     * @param secretKey The secret key for signing.
     * @return The constructed transaction wrapper.
     */
    LoanBrokerMergeInbox
    build(PublicKey const& publicKey, SecretKey const& secretKey)
    {
        sign(publicKey, secretKey);
        return LoanBrokerMergeInbox{std::make_shared<STTx>(std::move(object_))};
    }
};

}  // namespace xrpl::transactions
