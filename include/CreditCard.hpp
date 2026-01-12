#ifndef CREDITCARD_HPP
#define CREDITCARD_HPP

#include <string>
#include <memory>
#include "Card.hpp"

class BankAccount;

/**
 * @class CreditCard
 * @brief Represents a credit card with a credit limit and running balance.
 *
 * A CreditCard tracks:
 * - a credit limit (maximum allowed outstanding balance)
 * - a current outstanding balance
 * - optional linkage to a BankAccount (stored as a weak_ptr to avoid ownership cycles)
 * - optional contactless enable/disable flag
 *
 * @note In the current implementation, charges and payments typically update only the
 * card's internal balance. The linked bank account can be used for future integration.
 */
class CreditCard : public Card {
private:
    double credit_limit_;                     ///< Maximum credit allowed on the card (must be >= 0).
    double current_balance_;                  ///< Current outstanding balance owed on the card.
    std::weak_ptr<BankAccount> linked_account_; ///< Linked bank account (non-owning reference).
    bool contactless_enable_;                 ///< True if contactless payments are enabled.

public:
    /**
     * @brief Default constructor.
     *
     * Initializes:
     * - credit_limit_ = 0
     * - current_balance_ = 0
     * - contactless disabled
     * - no linked account
     *
     * @note A default-constructed card is typically not "valid" until a positive
     * credit limit is set and card details are properly populated (depending on Card::validate()).
     */
    CreditCard();

    /**
     * @brief Construct a CreditCard with a credit limit, activation state, and linked account.
     *
     * Typically generates card details (number/expiration/CVV) via the Card base class,
     * initializes balance to 0, and stores a weak reference to the linked account.
     *
     * @param credit_limit The credit limit assigned to the card (must be >= 0).
     * @param isActivated Whether the card is initially activated.
     * @param linked_account Shared pointer to the linked BankAccount (stored internally as weak_ptr).
     */
    CreditCard(double credit_limit,
               bool isActivated,
               std::shared_ptr<BankAccount> linked_account);

    // =========================
    // Getters
    // =========================

    /**
     * @brief Get the current outstanding balance.
     * @return Current balance owed on the credit card.
     */
    double getCurrentBalance() const;

    /**
     * @brief Get the credit limit.
     * @return Credit limit.
     */
    double getCreditLimit() const;

    /**
     * @brief Compute utilization percentage.
     *
     * Utilization is calculated as:
     *   (current_balance_ / credit_limit_) * 100
     *
     * Returns 0 if credit_limit_ <= 0 or current_balance_ <= 0.
     *
     * @return Utilization percentage in range [0, 100+] depending on state.
     */
    double getUtilizationPercentage() const;

    /**
     * @brief Get the linked bank account (weak pointer).
     * @return Weak pointer to the linked BankAccount.
     */
    std::weak_ptr<BankAccount> getLinkedAccount() const;

    /**
     * @brief Check if contactless payments are enabled.
     * @return True if enabled, false otherwise.
     */
    bool isContactlessEnabled() const;

    // =========================
    // Setters / Operations
    // =========================

    /**
     * @brief Set the credit limit.
     * @param limit New credit limit (must be >= 0).
     * @return True if set successfully, false if limit is negative.
     *
     * @note If you set the credit limit to 0, validate() will typically fail
     * if it requires credit_limit_ > 0.
     */
    bool setCreditLimit(double limit);

    /**
     * @brief Enable or disable contactless payments.
     * @param enable True to enable, false to disable.
     */
    void enableContactless(bool enable);

    /**
     * @brief Validate the credit card.
     *
     * Extends Card::validate() by also requiring the credit limit to be > 0.
     *
     * @return True if the card is valid, false otherwise.
     */
    bool validate() const override;

    /**
     * @brief Process a payment/charge on the card (Card override).
     *
     * In the current design, this typically delegates to charge(amount).
     *
     * @param amount Amount to charge.
     * @return True if processed successfully, false otherwise.
     */
    bool processPayment(double amount) override;

    /**
     * @brief Check if the card has expired (Card override).
     *
     * Typically delegates to Card::isExpired() which compares expiration date to current date.
     *
     * @return True if expired, false otherwise.
     */
    bool checkIfExpired() override;

    /**
     * @brief Charge a given amount to the credit card.
     *
     * The charge succeeds only if:
     * - the card is activated
     * - amount is positive
     * - current_balance_ + amount does not exceed credit_limit_
     *
     * @param amount Amount to charge.
     * @return True if charge succeeds, false otherwise.
     */
    bool charge(double amount);

    /**
     * @brief Make a payment to reduce the outstanding balance.
     *
     * Accepts only positive payment amounts.
     *
     * @param amount Payment amount.
     * @return True if accepted, false otherwise.
     *
     * @note If your .cpp subtracts directly, overpaying may make current_balance_ negative.
     * If you prefer, clamp the balance at 0 in the implementation.
     */
    bool makePayment(double amount);
};

#endif
