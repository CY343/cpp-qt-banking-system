#include "DebitCard.hpp"
#include "CheckingAccount.hpp"
#include "SecurityHelper.hpp"
#include <openssl/rand.h>
#include <iostream>
#include <string>
#include <stdexcept>

/**
 * @brief Called after a PIN change.
 *
 * Default behavior prints a security alert message.
 */
void DebitCard::onPinChange()
{
    std::cout << "Security alert: Pin changed for card " << getCardNumber() << std::endl;
}

/**
 * @brief Default constructor for DebitCard.
 *
 * Initializes:
 * - base Card as inactive, not expired
 * - empty salt and PIN hash
 * - limits/spend set to 0
 * - contactless disabled
 * - failed attempts = 0
 * - no linked account
 *
 * @note validate() will fail in this state because pin_hash_ is empty and limit is 0.
 */
DebitCard::DebitCard()
    : Card(false, false),
      salt_(""),
      pin_hash_(""),
      daily_withdrawal_limit_(0.0),
      daily_spend_amount_(0.0),
      contactless_enable_(false),
      failed_attempts_(0)
{}

/**
 * @brief Parameterized constructor for DebitCard.
 *
 * Creates an activated card (not expired), links it to an account, sets limits,
 * enables contactless, generates a Visa-style card number (prefix "4"),
 * and securely sets the PIN.
 *
 * If PIN is invalid, attempts to set a default PIN "0000".
 *
 * @param account Linked checking account.
 * @param daily_withdrawal_limit Daily withdrawal limit.
 * @param pin Initial PIN (4-6 digits).
 *
 * @throws std::runtime_error If OpenSSL reports insufficient entropy,
 * or if default PIN setup fails, or if resulting PIN hash is empty.
 */
DebitCard::DebitCard(std::shared_ptr<CheckingAccount> account,
                     double daily_withdrawal_limit,
                     const std::string& pin)
    : Card(true, false),
      daily_withdrawal_limit_(daily_withdrawal_limit),
      daily_spend_amount_(0.0),
      contactless_enable_(true),
      failed_attempts_(0),
      linked_debit_card_account_(account)
{
    // Generate and set a 16-digit card number with prefix "4" (Visa-style)
    std::string generatedNumber = generateCardNumberWithPrefix("4");
    setCardNumber(generatedNumber);

    // Check OpenSSL entropy pool status
    if (!RAND_status()) {
        throw std::runtime_error("Insufficient entropy for secure operations");
    }

    // Set PIN (fallback to default "0000" if invalid)
    if (!setPin(pin)) {
        std::cerr << "Invalid PIN! Setting default PIN\n";
        if (!setPin("0000")) {
            throw std::runtime_error("Failed to set default PIN");
        }
    }

    if (pin_hash_.empty()) {
        throw std::runtime_error("PIN hash empty after constructor");
    }
}

/**
 * @brief Get daily withdrawal limit.
 * @return Daily withdrawal limit.
 */
double DebitCard::getDailyWithdrawalLimit() const
{
    return daily_withdrawal_limit_;
}

/**
 * @brief Get daily spend amount (tracking field).
 * @return Amount spent today.
 */
double DebitCard::getDailySpendAmount() const
{
    return daily_spend_amount_;
}

/**
 * @brief Get linked checking account weak pointer.
 * @return Weak pointer to linked CheckingAccount.
 */
std::weak_ptr<CheckingAccount> DebitCard::getLinkedAccount() const
{
    return linked_debit_card_account_;
}

/**
 * @brief Set daily spend amount (tracking field).
 * @param amount New spend amount.
 * @return True if set successfully.
 */
bool DebitCard::SetDailySpendAmount(const double& amount)
{
    daily_spend_amount_ = amount;
    return true;
}

/**
 * @brief Set daily withdrawal limit.
 * @param limit New daily withdrawal limit.
 * @return True if set successfully.
 */
bool DebitCard::SetDailyWithdrawalLimit(const double& limit)
{
    daily_withdrawal_limit_ = limit;
    return true;
}

/**
 * @brief Securely set the PIN using a new random salt and stored hash.
 *
 * Accepts only PIN lengths 4-6 characters (length validation only).
 * Generates a new 16-byte salt and stores:
 * - salt_  (raw bytes stored in std::string)
 * - pin_hash_ = SecurityHelper::hashPin(pin, salt_)
 *
 * @param pin New PIN string.
 * @return True if accepted, false if invalid length.
 * @throws std::runtime_error If secure salt generation fails.
 */
bool DebitCard::setPin(const std::string pin)
{
    if (pin.size() < 4 || pin.size() > 6) {
        std::cout << "The pin must be within 4 to 6 digits" << std::endl;
        return false;
    }

    unsigned char new_salt[16];
    if (RAND_bytes(new_salt, sizeof(new_salt)) != 1) {
        throw std::runtime_error("Failed to generate secure salt");
    }

    salt_ = std::string(reinterpret_cast<char*>(new_salt), sizeof(new_salt));

    // Store salted hash of the PIN
    pin_hash_ = SecurityHelper::hashPin(pin, salt_);
    return true;
}

/**
 * @brief Change PIN after verifying the old PIN.
 *
 * @param old_pin Current PIN.
 * @param new_pin New PIN.
 * @return True if changed successfully.
 */
bool DebitCard::changePin(const std::string old_pin, const std::string new_pin)
{
    if (!verifyPin(old_pin)) {
        std::cerr << "Old PIN verification failed\n";
        return false;
    }

    if (setPin(new_pin)) {
        onPinChange();
        return true;
    }

    return false;
}

/**
 * @brief Process a payment by withdrawing funds from the linked account.
 *
 * Requirements:
 * - card must be activated
 * - linked account must exist
 * - amount must not exceed account balance
 *
 * @param amount Amount to withdraw.
 * @return True if withdrawal succeeded, false otherwise.
 */
bool DebitCard::processPayment(double amount)
{
    if (!isActivated()) return false;

    auto account = linked_debit_card_account_.lock();
    if (!account) return false;

    if (amount > account->getAccountBalance()) {
        return false;
    }

    return account->applyWithdraw(amount);
}

/**
 * @brief Validate debit card state.
 *
 * Extends Card::validate() by requiring:
 * - PIN hash must be non-empty
 * - daily withdrawal limit must be > 0
 *
 * @return True if valid, false otherwise.
 */
bool DebitCard::validate() const
{
    return Card::validate() && !pin_hash_.empty() && daily_withdrawal_limit_ > 0;
}

/**
 * @brief DebitCard expiration check implementation.
 *
 * Delegates to Card::isExpired() which checks expiration date against current date.
 *
 * @return True if expired.
 */
bool DebitCard::checkIfExpired()
{
    return isExpired();
}

/**
 * @brief Check whether contactless payment is enabled.
 * @return True if enabled, false otherwise.
 */
bool DebitCard::isContactlessEnabled() const
{
    return contactless_enable_;
}

/**
 * @brief Enable/disable contactless payment support.
 * @param enable True to enable, false to disable.
 */
void DebitCard::enableContactless(bool enable)
{
    contactless_enable_ = enable;
}

/**
 * @brief Verify a PIN attempt using the stored salt + hash.
 *
 * Allows up to 3 failed attempts. After 3 failures, card is deactivated.
 *
 * @param attempt PIN attempt.
 * @return True if PIN correct, false otherwise.
 */
bool DebitCard::verifyPin(const std::string& attempt) const
{
    if (failed_attempts_ >= 3) {
        std::cerr << "Card locked due to too many failed attempts\n";
        return false;
    }

    const std::string attempt_hash = SecurityHelper::hashPin(attempt, salt_);

    if (attempt_hash != pin_hash_) {
        failed_attempts_++;

        if (failed_attempts_ >= 3) {
            std::cerr << "Card locked due to too many failed attempts" << std::endl;
            const_cast<DebitCard*>(this)->setActivated(false);
        }

        return false;
    }

    failed_attempts_ = 0;
    return true;
}

/**
 * @brief Generate a secure random 16-digit card number using a prefix.
 *
 * @param prefix Starting digits (e.g., "4").
 * @return Card number string of length 16.
 * @throws std::runtime_error If secure random generation fails.
 */
std::string DebitCard::generateCardNumberWithPrefix(const std::string& prefix)
{
    std::string cardNumber = prefix;

    while (cardNumber.size() < 16) {
        unsigned char digit;
        if (RAND_bytes(&digit, 1) != 1) {
            throw std::runtime_error("Failed to generate secure random digit");
        }
        cardNumber += static_cast<char>('0' + (digit % 10));
    }

    return cardNumber;
}

/**
 * @brief Get the stored plain PIN value.
 *
 * @note pin_ is not assigned in setPin() in the current implementation,
 * so this likely returns an empty string.
 *
 * @return pin_.
 */
std::string DebitCard::getPin() const
{
    return pin_;
}
