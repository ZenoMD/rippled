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

class LoanSetBuilder;

/**
 * @brief Transaction: LoanSet
 *
 * Type: ttLOAN_SET (80)
 * Delegable: Delegation::NotDelegable
 * Amendment: featureLendingProtocol
 * Privileges: Privilege::MayAuthorizeMpt | Privilege::MayModifyVault
 *
 * Immutable wrapper around STTx providing type-safe field access.
 * Use LoanSetBuilder to construct new transactions.
 */
class LoanSet : public TransactionBase
{
public:
    static constexpr xrpl::TxType txType = ttLOAN_SET;

    /**
     * @brief Construct a LoanSet transaction wrapper from an existing STTx object.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    explicit LoanSet(std::shared_ptr<STTx const> tx)
        : TransactionBase(std::move(tx))
    {
        // Verify transaction type
        if (tx_->getTxnType() != txType)
        {
            throw std::runtime_error("Invalid transaction type for LoanSet");
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
     * @brief Get sfData (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getData() const
    {
        if (hasData())
        {
            return this->tx_->at(sfData);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfData is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasData() const
    {
        return this->tx_->isFieldPresent(sfData);
    }

    /**
     * @brief Get sfCounterparty (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_ACCOUNT::type::value_type>
    getCounterparty() const
    {
        if (hasCounterparty())
        {
            return this->tx_->at(sfCounterparty);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfCounterparty is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasCounterparty() const
    {
        return this->tx_->isFieldPresent(sfCounterparty);
    }
    /**
     * @brief Get sfCounterpartySignature (SoeOptional)
     * @note This is an untyped field.
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    std::optional<STObject>
    getCounterpartySignature() const
    {
        if (this->tx_->isFieldPresent(sfCounterpartySignature))
            return this->tx_->getFieldObject(sfCounterpartySignature);
        return std::nullopt;
    }

    /**
     * @brief Check if sfCounterpartySignature is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasCounterpartySignature() const
    {
        return this->tx_->isFieldPresent(sfCounterpartySignature);
    }

    /**
     * @brief Get sfLoanOriginationFee (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_NUMBER::type::value_type>
    getLoanOriginationFee() const
    {
        if (hasLoanOriginationFee())
        {
            return this->tx_->at(sfLoanOriginationFee);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfLoanOriginationFee is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasLoanOriginationFee() const
    {
        return this->tx_->isFieldPresent(sfLoanOriginationFee);
    }

    /**
     * @brief Get sfLoanServiceFee (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_NUMBER::type::value_type>
    getLoanServiceFee() const
    {
        if (hasLoanServiceFee())
        {
            return this->tx_->at(sfLoanServiceFee);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfLoanServiceFee is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasLoanServiceFee() const
    {
        return this->tx_->isFieldPresent(sfLoanServiceFee);
    }

    /**
     * @brief Get sfLatePaymentFee (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_NUMBER::type::value_type>
    getLatePaymentFee() const
    {
        if (hasLatePaymentFee())
        {
            return this->tx_->at(sfLatePaymentFee);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfLatePaymentFee is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasLatePaymentFee() const
    {
        return this->tx_->isFieldPresent(sfLatePaymentFee);
    }

    /**
     * @brief Get sfClosePaymentFee (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_NUMBER::type::value_type>
    getClosePaymentFee() const
    {
        if (hasClosePaymentFee())
        {
            return this->tx_->at(sfClosePaymentFee);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfClosePaymentFee is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasClosePaymentFee() const
    {
        return this->tx_->isFieldPresent(sfClosePaymentFee);
    }

    /**
     * @brief Get sfOverpaymentFee (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_UINT32::type::value_type>
    getOverpaymentFee() const
    {
        if (hasOverpaymentFee())
        {
            return this->tx_->at(sfOverpaymentFee);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfOverpaymentFee is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasOverpaymentFee() const
    {
        return this->tx_->isFieldPresent(sfOverpaymentFee);
    }

    /**
     * @brief Get sfInterestRate (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_UINT32::type::value_type>
    getInterestRate() const
    {
        if (hasInterestRate())
        {
            return this->tx_->at(sfInterestRate);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfInterestRate is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasInterestRate() const
    {
        return this->tx_->isFieldPresent(sfInterestRate);
    }

    /**
     * @brief Get sfLateInterestRate (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_UINT32::type::value_type>
    getLateInterestRate() const
    {
        if (hasLateInterestRate())
        {
            return this->tx_->at(sfLateInterestRate);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfLateInterestRate is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasLateInterestRate() const
    {
        return this->tx_->isFieldPresent(sfLateInterestRate);
    }

    /**
     * @brief Get sfCloseInterestRate (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_UINT32::type::value_type>
    getCloseInterestRate() const
    {
        if (hasCloseInterestRate())
        {
            return this->tx_->at(sfCloseInterestRate);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfCloseInterestRate is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasCloseInterestRate() const
    {
        return this->tx_->isFieldPresent(sfCloseInterestRate);
    }

    /**
     * @brief Get sfOverpaymentInterestRate (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_UINT32::type::value_type>
    getOverpaymentInterestRate() const
    {
        if (hasOverpaymentInterestRate())
        {
            return this->tx_->at(sfOverpaymentInterestRate);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfOverpaymentInterestRate is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasOverpaymentInterestRate() const
    {
        return this->tx_->isFieldPresent(sfOverpaymentInterestRate);
    }

    /**
     * @brief Get sfPrincipalRequested (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_NUMBER::type::value_type>
    getPrincipalRequested() const
    {
        if (hasPrincipalRequested())
        {
            return this->tx_->at(sfPrincipalRequested);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfPrincipalRequested is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasPrincipalRequested() const
    {
        return this->tx_->isFieldPresent(sfPrincipalRequested);
    }

    /**
     * @brief Get sfPaymentTotal (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_UINT32::type::value_type>
    getPaymentTotal() const
    {
        if (hasPaymentTotal())
        {
            return this->tx_->at(sfPaymentTotal);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfPaymentTotal is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasPaymentTotal() const
    {
        return this->tx_->isFieldPresent(sfPaymentTotal);
    }

    /**
     * @brief Get sfPaymentInterval (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_UINT32::type::value_type>
    getPaymentInterval() const
    {
        if (hasPaymentInterval())
        {
            return this->tx_->at(sfPaymentInterval);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfPaymentInterval is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasPaymentInterval() const
    {
        return this->tx_->isFieldPresent(sfPaymentInterval);
    }

    /**
     * @brief Get sfGracePeriod (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_UINT32::type::value_type>
    getGracePeriod() const
    {
        if (hasGracePeriod())
        {
            return this->tx_->at(sfGracePeriod);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfGracePeriod is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasGracePeriod() const
    {
        return this->tx_->isFieldPresent(sfGracePeriod);
    }

    /**
     * @brief Get sfSenderEncryptedAmount (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getSenderEncryptedAmount() const
    {
        if (hasSenderEncryptedAmount())
        {
            return this->tx_->at(sfSenderEncryptedAmount);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfSenderEncryptedAmount is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasSenderEncryptedAmount() const
    {
        return this->tx_->isFieldPresent(sfSenderEncryptedAmount);
    }

    /**
     * @brief Get sfDestinationEncryptedAmount (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getDestinationEncryptedAmount() const
    {
        if (hasDestinationEncryptedAmount())
        {
            return this->tx_->at(sfDestinationEncryptedAmount);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfDestinationEncryptedAmount is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasDestinationEncryptedAmount() const
    {
        return this->tx_->isFieldPresent(sfDestinationEncryptedAmount);
    }

    /**
     * @brief Get sfIssuerEncryptedAmount (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getIssuerEncryptedAmount() const
    {
        if (hasIssuerEncryptedAmount())
        {
            return this->tx_->at(sfIssuerEncryptedAmount);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfIssuerEncryptedAmount is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasIssuerEncryptedAmount() const
    {
        return this->tx_->isFieldPresent(sfIssuerEncryptedAmount);
    }

    /**
     * @brief Get sfAuditorEncryptedAmount (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getAuditorEncryptedAmount() const
    {
        if (hasAuditorEncryptedAmount())
        {
            return this->tx_->at(sfAuditorEncryptedAmount);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfAuditorEncryptedAmount is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasAuditorEncryptedAmount() const
    {
        return this->tx_->isFieldPresent(sfAuditorEncryptedAmount);
    }

    /**
     * @brief Get sfBalanceCommitment (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getBalanceCommitment() const
    {
        if (hasBalanceCommitment())
        {
            return this->tx_->at(sfBalanceCommitment);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfBalanceCommitment is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasBalanceCommitment() const
    {
        return this->tx_->isFieldPresent(sfBalanceCommitment);
    }

    /**
     * @brief Get sfAmountCommitment (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getAmountCommitment() const
    {
        if (hasAmountCommitment())
        {
            return this->tx_->at(sfAmountCommitment);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfAmountCommitment is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasAmountCommitment() const
    {
        return this->tx_->isFieldPresent(sfAmountCommitment);
    }

    /**
     * @brief Get sfZKProof (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getZKProof() const
    {
        if (hasZKProof())
        {
            return this->tx_->at(sfZKProof);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfZKProof is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasZKProof() const
    {
        return this->tx_->isFieldPresent(sfZKProof);
    }

    /**
     * @brief Get sfLoanAuditorEncryptionKey (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getLoanAuditorEncryptionKey() const
    {
        if (hasLoanAuditorEncryptionKey())
        {
            return this->tx_->at(sfLoanAuditorEncryptionKey);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfLoanAuditorEncryptionKey is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasLoanAuditorEncryptionKey() const
    {
        return this->tx_->isFieldPresent(sfLoanAuditorEncryptionKey);
    }

    /**
     * @brief Get sfLoanAuditorEncryptedPrincipal (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getLoanAuditorEncryptedPrincipal() const
    {
        if (hasLoanAuditorEncryptedPrincipal())
        {
            return this->tx_->at(sfLoanAuditorEncryptedPrincipal);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfLoanAuditorEncryptedPrincipal is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasLoanAuditorEncryptedPrincipal() const
    {
        return this->tx_->isFieldPresent(sfLoanAuditorEncryptedPrincipal);
    }

    /**
     * @brief Get sfBorrowerEncryptedAmountOwed (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getBorrowerEncryptedAmountOwed() const
    {
        if (hasBorrowerEncryptedAmountOwed())
        {
            return this->tx_->at(sfBorrowerEncryptedAmountOwed);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfBorrowerEncryptedAmountOwed is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasBorrowerEncryptedAmountOwed() const
    {
        return this->tx_->isFieldPresent(sfBorrowerEncryptedAmountOwed);
    }

    /**
     * @brief Get sfBrokerEncryptedAmountOwed (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getBrokerEncryptedAmountOwed() const
    {
        if (hasBrokerEncryptedAmountOwed())
        {
            return this->tx_->at(sfBrokerEncryptedAmountOwed);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfBrokerEncryptedAmountOwed is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasBrokerEncryptedAmountOwed() const
    {
        return this->tx_->isFieldPresent(sfBrokerEncryptedAmountOwed);
    }

    /**
     * @brief Get sfLoanAuditorEncryptedAmountOwed (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getLoanAuditorEncryptedAmountOwed() const
    {
        if (hasLoanAuditorEncryptedAmountOwed())
        {
            return this->tx_->at(sfLoanAuditorEncryptedAmountOwed);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfLoanAuditorEncryptedAmountOwed is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasLoanAuditorEncryptedAmountOwed() const
    {
        return this->tx_->isFieldPresent(sfLoanAuditorEncryptedAmountOwed);
    }

    /**
     * @brief Get sfBorrowerEncryptedPeriodicPayment (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getBorrowerEncryptedPeriodicPayment() const
    {
        if (hasBorrowerEncryptedPeriodicPayment())
        {
            return this->tx_->at(sfBorrowerEncryptedPeriodicPayment);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfBorrowerEncryptedPeriodicPayment is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasBorrowerEncryptedPeriodicPayment() const
    {
        return this->tx_->isFieldPresent(sfBorrowerEncryptedPeriodicPayment);
    }

    /**
     * @brief Get sfBrokerEncryptedPeriodicPayment (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getBrokerEncryptedPeriodicPayment() const
    {
        if (hasBrokerEncryptedPeriodicPayment())
        {
            return this->tx_->at(sfBrokerEncryptedPeriodicPayment);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfBrokerEncryptedPeriodicPayment is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasBrokerEncryptedPeriodicPayment() const
    {
        return this->tx_->isFieldPresent(sfBrokerEncryptedPeriodicPayment);
    }

    /**
     * @brief Get sfLoanAuditorEncryptedPeriodicPayment (SoeOptional)
     * @return The field value, or std::nullopt if not present.
     */
    [[nodiscard]]
    protocol_autogen::Optional<SF_VL::type::value_type>
    getLoanAuditorEncryptedPeriodicPayment() const
    {
        if (hasLoanAuditorEncryptedPeriodicPayment())
        {
            return this->tx_->at(sfLoanAuditorEncryptedPeriodicPayment);
        }
        return std::nullopt;
    }

    /**
     * @brief Check if sfLoanAuditorEncryptedPeriodicPayment is present.
     * @return True if the field is present, false otherwise.
     */
    [[nodiscard]]
    bool
    hasLoanAuditorEncryptedPeriodicPayment() const
    {
        return this->tx_->isFieldPresent(sfLoanAuditorEncryptedPeriodicPayment);
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
};

/**
 * @brief Builder for LoanSet transactions.
 *
 * Provides a fluent interface for constructing transactions with method chaining.
 * Uses STObject internally for flexible transaction construction.
 * Inherits common field setters from TransactionBuilderBase.
 */
class LoanSetBuilder : public TransactionBuilderBase<LoanSetBuilder>
{
public:
    /**
     * @brief Construct a new LoanSetBuilder with required fields.
     * @param account The account initiating the transaction.
     * @param loanBrokerID The sfLoanBrokerID field value.
     * @param sequence Optional sequence number for the transaction.
     * @param fee Optional fee for the transaction.
     */
    LoanSetBuilder(SF_ACCOUNT::type::value_type account,
                     std::decay_t<typename SF_UINT256::type::value_type> const& loanBrokerID,                    std::optional<SF_UINT32::type::value_type> sequence = std::nullopt,
                    std::optional<SF_AMOUNT::type::value_type> fee = std::nullopt
)
        : TransactionBuilderBase<LoanSetBuilder>(ttLOAN_SET, account, sequence, fee)
    {
        setLoanBrokerID(loanBrokerID);
    }

    /**
     * @brief Construct a LoanSetBuilder from an existing STTx object.
     * @param tx The existing transaction to copy from.
     * @throws std::runtime_error if the transaction type doesn't match.
     */
    LoanSetBuilder(std::shared_ptr<STTx const> tx)
    {
        if (tx->getTxnType() != ttLOAN_SET)
        {
            throw std::runtime_error("Invalid transaction type for LoanSetBuilder");
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
    LoanSetBuilder&
    setLoanBrokerID(std::decay_t<typename SF_UINT256::type::value_type> const& value)
    {
        object_[sfLoanBrokerID] = value;
        return *this;
    }

    /**
     * @brief Set sfData (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setData(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfData] = value;
        return *this;
    }

    /**
     * @brief Set sfCounterparty (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setCounterparty(std::decay_t<typename SF_ACCOUNT::type::value_type> const& value)
    {
        object_[sfCounterparty] = value;
        return *this;
    }

    /**
     * @brief Set sfCounterpartySignature (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setCounterpartySignature(STObject const& value)
    {
        object_.setFieldObject(sfCounterpartySignature, value);
        return *this;
    }

    /**
     * @brief Set sfLoanOriginationFee (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setLoanOriginationFee(std::decay_t<typename SF_NUMBER::type::value_type> const& value)
    {
        object_[sfLoanOriginationFee] = value;
        return *this;
    }

    /**
     * @brief Set sfLoanServiceFee (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setLoanServiceFee(std::decay_t<typename SF_NUMBER::type::value_type> const& value)
    {
        object_[sfLoanServiceFee] = value;
        return *this;
    }

    /**
     * @brief Set sfLatePaymentFee (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setLatePaymentFee(std::decay_t<typename SF_NUMBER::type::value_type> const& value)
    {
        object_[sfLatePaymentFee] = value;
        return *this;
    }

    /**
     * @brief Set sfClosePaymentFee (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setClosePaymentFee(std::decay_t<typename SF_NUMBER::type::value_type> const& value)
    {
        object_[sfClosePaymentFee] = value;
        return *this;
    }

    /**
     * @brief Set sfOverpaymentFee (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setOverpaymentFee(std::decay_t<typename SF_UINT32::type::value_type> const& value)
    {
        object_[sfOverpaymentFee] = value;
        return *this;
    }

    /**
     * @brief Set sfInterestRate (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setInterestRate(std::decay_t<typename SF_UINT32::type::value_type> const& value)
    {
        object_[sfInterestRate] = value;
        return *this;
    }

    /**
     * @brief Set sfLateInterestRate (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setLateInterestRate(std::decay_t<typename SF_UINT32::type::value_type> const& value)
    {
        object_[sfLateInterestRate] = value;
        return *this;
    }

    /**
     * @brief Set sfCloseInterestRate (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setCloseInterestRate(std::decay_t<typename SF_UINT32::type::value_type> const& value)
    {
        object_[sfCloseInterestRate] = value;
        return *this;
    }

    /**
     * @brief Set sfOverpaymentInterestRate (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setOverpaymentInterestRate(std::decay_t<typename SF_UINT32::type::value_type> const& value)
    {
        object_[sfOverpaymentInterestRate] = value;
        return *this;
    }

    /**
     * @brief Set sfPrincipalRequested (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setPrincipalRequested(std::decay_t<typename SF_NUMBER::type::value_type> const& value)
    {
        object_[sfPrincipalRequested] = value;
        return *this;
    }

    /**
     * @brief Set sfPaymentTotal (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setPaymentTotal(std::decay_t<typename SF_UINT32::type::value_type> const& value)
    {
        object_[sfPaymentTotal] = value;
        return *this;
    }

    /**
     * @brief Set sfPaymentInterval (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setPaymentInterval(std::decay_t<typename SF_UINT32::type::value_type> const& value)
    {
        object_[sfPaymentInterval] = value;
        return *this;
    }

    /**
     * @brief Set sfGracePeriod (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setGracePeriod(std::decay_t<typename SF_UINT32::type::value_type> const& value)
    {
        object_[sfGracePeriod] = value;
        return *this;
    }

    /**
     * @brief Set sfSenderEncryptedAmount (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setSenderEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfSenderEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfDestinationEncryptedAmount (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setDestinationEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfDestinationEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfIssuerEncryptedAmount (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setIssuerEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfIssuerEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfAuditorEncryptedAmount (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setAuditorEncryptedAmount(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfAuditorEncryptedAmount] = value;
        return *this;
    }

    /**
     * @brief Set sfBalanceCommitment (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setBalanceCommitment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfBalanceCommitment] = value;
        return *this;
    }

    /**
     * @brief Set sfAmountCommitment (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setAmountCommitment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfAmountCommitment] = value;
        return *this;
    }

    /**
     * @brief Set sfZKProof (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setZKProof(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfZKProof] = value;
        return *this;
    }

    /**
     * @brief Set sfLoanAuditorEncryptionKey (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setLoanAuditorEncryptionKey(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfLoanAuditorEncryptionKey] = value;
        return *this;
    }

    /**
     * @brief Set sfLoanAuditorEncryptedPrincipal (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setLoanAuditorEncryptedPrincipal(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfLoanAuditorEncryptedPrincipal] = value;
        return *this;
    }

    /**
     * @brief Set sfBorrowerEncryptedAmountOwed (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setBorrowerEncryptedAmountOwed(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfBorrowerEncryptedAmountOwed] = value;
        return *this;
    }

    /**
     * @brief Set sfBrokerEncryptedAmountOwed (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setBrokerEncryptedAmountOwed(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfBrokerEncryptedAmountOwed] = value;
        return *this;
    }

    /**
     * @brief Set sfLoanAuditorEncryptedAmountOwed (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setLoanAuditorEncryptedAmountOwed(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfLoanAuditorEncryptedAmountOwed] = value;
        return *this;
    }

    /**
     * @brief Set sfBorrowerEncryptedPeriodicPayment (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setBorrowerEncryptedPeriodicPayment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfBorrowerEncryptedPeriodicPayment] = value;
        return *this;
    }

    /**
     * @brief Set sfBrokerEncryptedPeriodicPayment (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setBrokerEncryptedPeriodicPayment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfBrokerEncryptedPeriodicPayment] = value;
        return *this;
    }

    /**
     * @brief Set sfLoanAuditorEncryptedPeriodicPayment (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setLoanAuditorEncryptedPeriodicPayment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfLoanAuditorEncryptedPeriodicPayment] = value;
        return *this;
    }

    /**
     * @brief Set sfAmountOwedCommitment (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setAmountOwedCommitment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfAmountOwedCommitment] = value;
        return *this;
    }

    /**
     * @brief Set sfPeriodicPaymentCommitment (SoeOptional)
     * @return Reference to this builder for method chaining.
     */
    LoanSetBuilder&
    setPeriodicPaymentCommitment(std::decay_t<typename SF_VL::type::value_type> const& value)
    {
        object_[sfPeriodicPaymentCommitment] = value;
        return *this;
    }

    /**
     * @brief Build and return the LoanSet wrapper.
     * @param publicKey The public key for signing.
     * @param secretKey The secret key for signing.
     * @return The constructed transaction wrapper.
     */
    LoanSet
    build(PublicKey const& publicKey, SecretKey const& secretKey)
    {
        sign(publicKey, secretKey);
        return LoanSet{std::make_shared<STTx>(std::move(object_))};
    }
};

}  // namespace xrpl::transactions
