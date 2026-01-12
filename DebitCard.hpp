#ifndef DEBITCARD_HPP
#define DEBITCARD_HPP

#include "CheckingAccount.hpp"
#include "SecurityHelper.hpp"
#include "Card.hpp"
#include <iostream>
#include <string>
#include <memory>

class CheckingAccount;
class BankAccount;

/**
 * @class DebitCard
 * @brief Represents a debit card linked to a checking account.
 *
 * A DebitCard withdraws money directly from a linked CheckingAccount.
 * It supports:
 * - secure PIN storage using salt + hash
 * - daily withdrawal/spend tracking fields
 * - optional contactless support
 * - lockout after too many failed PIN attempts
 *
 * @note This class derives from Card and overrides key virtual functions such as
 * processPayment(), validate(), and checkIfExpired().
 */
class DebitCard : public Card {
private:
    std::string salt_;                   ///< Random salt used for hashing the PIN.
    std::string pin_hash_;               ///< Hashed PIN (hash(pin + salt)).
    double daily_withdrawal_limit_;      ///< Maximum withdrawal allowed per day.
    double daily_spend_amount_;          ///< Amount spent so far today (tracking field).
    bool contactless_enable_;            ///< True if contactless payments are enabled.

    /**
     * @brief Number of failed PIN attempts.
     *
     * Marked mutable so verifyPin() can update it even though verifyPin() is const.
     * After 3 failures, the card is deactivated (locked).
     */
    mutable int failed_attempts_ = 0;

    /**
     * @brief Linked checking account (weak pointer).
     *
     * weak_ptr avoids ownership cycles (the account can exist without being owned by the card).
     * Use lock() to access safely.
     */
    std::weak_ptr<CheckingAccount> linked_debit_card_account_;

    /**
     * @brief Plain PIN storage (currently unused).
     *
     * @note In the current implementation, setPin() does NOT assign to pin_,
     * so getPin() will return an empty/default value unless you set it elsewhere.
     * Keeping this field is not recommended for security in production.
     */
    std::string pin_;

protected:
    /**
     * @brief Hook that runs after a PIN change.
     *
     * Default implementation prints a security alert. Derived classes may override.
     */
    virtual void onPinChange();

public:
    /**
     * @brief Default constructor.
     *
     * Creates a card in a default state:
     * - not activated
     * - not expired
     * - no linked checking account
     * - limits/spend default to 0
     * - contactless disabled
     *
     * @note validate() will fail in this default state because
     * daily_withdrawal_limit_ is 0 and pin_hash_ is empty.
     */
    DebitCard();

    /**
     * @brief Construct a DebitCard linked to a CheckingAccount.
     *
     * Activates the card, sets daily withdrawal limit, enables contactless by default,
     * generates a Visa-like card number prefix ("4"), and securely sets the PIN.
     *
     * If the PIN is invalid, it attempts to set a default PIN of "0000".
     *
     * @param account Linked CheckingAccount.
     * @param daily_withdrawal_limit The daily withdrawal limit.
     * @param pin Initial PIN (4-6 digits).
     *
     * @throws std::runtime_error If OpenSSL reports insufficient entropy or if salt/PIN setup fails.
     */
    DebitCard(std::shared_ptr<CheckingAccount> account,
              double daily_withdrawal_limit,
              const std::string& pin);

    /**
     * @brief Get the daily withdrawal limit.
     * @return Daily withdrawal limit.
     */
    double getDailyWithdrawalLimit() const;

    /**
     * @brief Get the amount spent today (tracking field).
     * @return Daily spent amount.
     */
    double getDailySpendAmount() const;

    /**
     * @brief Get the linked checking account (weak pointer).
     * @return Weak pointer to linked CheckingAccount.
     */
    std::weak_ptr<CheckingAccount> getLinkedAccount() const;

    /**
     * @brief Check whether contactless payments are enabled.
     * @return True if enabled.
     */
    bool isContactlessEnabled() const;

    /**
     * @brief Enable/disable contactless payments.
     * @param enable True to enable, false to disable.
     */
    void enableContactless(bool enable);

    /**
     * @brief Set the daily spend amount (tracking field).
     * @param amount New daily spend amount.
     * @return True if set successfully.
     */
    bool SetDailySpendAmount(const double& amount);

    /**
     * @brief Set the daily withdrawal limit.
     * @param limit New daily withdrawal limit.
     * @return True if set successfully.
     */
    bool SetDailyWithdrawalLimit(const double& limit);

    /**
     * @brief Set the card PIN (stored as salted hash).
     *
     * Valid PIN length is 4 to 6 digits (length check only).
     *
     * @param pin New PIN.
     * @return True if PIN accepted and stored, false if invalid length.
     * @throws std::runtime_error If secure salt generation fails.
     */
    bool setPin(const std::string pin);

    /**
     * @brief Change PIN after verifying the current PIN.
     *
     * If old PIN verification passes and new PIN is valid,
     * updates the salted hash and calls onPinChange().
     *
     * @param old_pin Current PIN.
     * @param new_pin New PIN to set.
     * @return True if changed successfully, false otherwise.
     */
    bool changePin(const std::string old_pin, const std::string new_pin);

    /**
     * @brief Process a payment by withdrawing funds from the linked account.
     *
     * Requires the card to be activated and a valid linked account.
     * Fails if amount exceeds account balance.
     *
     * @param amount Amount to withdraw.
     * @return True if withdrawal succeeds, false otherwise.
     */
    bool processPayment(double amount) override;

    /**
     * @brief Validate debit card state.
     *
     * Extends Card::validate() by requiring:
     * - non-empty PIN hash
     * - daily withdrawal limit > 0
     *
     * @return True if valid.
     */
    bool validate() const override;

    /**
     * @brief Derived expiration check.
     * @return True if expired.
     */
    bool checkIfExpired() override;

    /**
     * @brief Verify a PIN attempt against stored salted hash.
     *
     * Allows up to 3 failed attempts. After 3 failures, the card is deactivated.
     *
     * @param attempt PIN attempt.
     * @return True if PIN matches, false otherwise.
     */
    bool verifyPin(const std::string& attempt) const;

    /**
     * @brief Generate a 16-digit card number starting with a prefix.
     *
     * Uses OpenSSL RAND_bytes for randomness.
     *
     * @param prefix Starting digit(s), e.g. "4" for Visa-style.
     * @return 16-digit card number string.
     * @throws std::runtime_error If random generation fails.
     */
    std::string generateCardNumberWithPrefix(const std::string& prefix);

    /**
     * @brief Get the stored plain PIN value (currently unused).
     *
     * @note In current implementation, pin_ is not set inside setPin(),
     * so this will likely be empty unless managed elsewhere.
     *
     * @return pin_ string.
     */
    std::string getPin() const;
};

#endif
