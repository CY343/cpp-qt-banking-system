#include "CreditCard.hpp"
#include "BankAccount.hpp"
#include <string>

/**
 * @brief Default constructor for CreditCard.
 *
 * Initializes a CreditCard in a default state:
 * - Card base is created inactive and not expired (Card(false, false))
 * - credit limit = 0
 * - balance = 0
 * - no linked account
 * - contactless disabled
 *
 * @note A default-constructed CreditCard typically fails validate() until
 * a positive credit limit is set and card fields are populated (depending on Card::validate()).
 */
CreditCard::CreditCard()
    : Card(false, false),
      credit_limit_(0.0),
      current_balance_(0.0),
      linked_account_(),
      contactless_enable_(false)
{}

/**
 * @brief Constructs a CreditCard with specified credit limit, activation status, and linked bank account.
 *
 * Creates card details using a BIN-style prefix "5" (commonly used for Mastercard-style examples),
 * sets the credit limit, initializes balance to 0, and stores a weak link to the provided BankAccount.
 *
 * @param credit_limit The credit limit for this credit card (must be >= 0).
 * @param isActivated Whether the card is initially activated.
 * @param linked_account Shared pointer to the linked BankAccount (stored internally as weak_ptr).
 *
 * @note This constructor generates card number/expiration/CVV. If your Card("5", true)
 * already generates them, then the explicit setCardNumber/setCvv/setExpiration calls below
 * overwrite those generated values.
 */
CreditCard::CreditCard(double credit_limit,
                       bool isActivated,
                       std::shared_ptr<BankAccount> linked_account)
    : Card("5", true),
      credit_limit_(credit_limit),
      current_balance_(0.0),
      linked_account_(linked_account),
      contactless_enable_(false)
{
    setActivated(isActivated);

    // might override card constructor generated values
    setCardNumber(generateCardNumber("5"));
    setCvv(generateCVV());
    setExpiration(generateExpiryDate(3));
}

/**
 * @brief Retrieves the current outstanding balance.
 * @return The current balance owed on the credit card.
 */
double CreditCard::getCurrentBalance() const
{
    return current_balance_;
}

/**
 * @brief Sets the credit limit for the credit card.
 *
 * @param limit The new credit limit to set (must be >= 0).
 * @return true if the limit is valid and successfully set, false if limit is negative.
 */
bool CreditCard::setCreditLimit(double limit)
{
    if (limit < 0) return false;
    credit_limit_ = limit;
    return true;
}

/**
 * @brief Retrieves a weak pointer to the linked bank account.
 * @return Weak pointer to the linked BankAccount.
 */
std::weak_ptr<BankAccount> CreditCard::getLinkedAccount() const
{
    return linked_account_;
}

/**
 * @brief Retrieves the credit limit.
 * @return The current credit limit.
 */
double CreditCard::getCreditLimit() const
{
    return credit_limit_;
}

/**
 * @brief Calculates the credit utilization percentage.
 *
 * Utilization is defined as:
 *   (current_balance / credit_limit) * 100
 *
 * Returns 0 if credit limit <= 0 or current balance <= 0.
 *
 * @return The utilization percentage.
 */
double CreditCard::getUtilizationPercentage() const
{
    if (credit_limit_ <= 0.0 || current_balance_ <= 0.0) return 0.0;
    return (current_balance_ / credit_limit_) * 100.0;
}

/**
 * @brief Validates the credit card.
 *
 * Uses Card::validate() and additionally requires credit_limit_ > 0.
 *
 * @return true if the card is valid, false otherwise.
 */
bool CreditCard::validate() const
{
    return Card::validate() && (credit_limit_ > 0.0);
}

/**
 * @brief Processes a payment/charge using the credit card.
 *
 * Delegates to charge(amount).
 *
 * @param amount Amount to charge.
 * @return true if charge succeeds, false otherwise.
 */
bool CreditCard::processPayment(double amount)
{
    return charge(amount);
}

/**
 * @brief Checks if the credit card has expired.
 *
 * Delegates to Card::isExpired() which compares expiration date to the current date.
 *
 * @return true if expired, false otherwise.
 */
bool CreditCard::checkIfExpired()
{
    return isExpired();
}

/**
 * @brief Attempts to charge the credit card a specified amount.
 *
 * Charge succeeds only if:
 * - card is activated
 * - amount > 0
 * - current_balance_ + amount <= credit_limit_
 *
 * @param amount Amount to charge.
 * @return true if charge succeeds, false otherwise.
 */
bool CreditCard::charge(double amount)
{
    if (!isActivated()) return false;
    if (amount <= 0.0) return false;
    if (current_balance_ + amount > credit_limit_) return false;

    current_balance_ += amount;
    return true;
}

/**
 * @brief Makes a payment to reduce the outstanding balance.
 *
 * @param amount Payment amount (must be positive).
 * @return true if accepted, false otherwise.
 *
 * @note In the current implementation, overpaying can make the balance negative.
 * If you want to prevent that, clamp at 0 in the implementation.
 */
bool CreditCard::makePayment(double amount)
{
    if (amount <= 0.0) return false;
    current_balance_ -= amount;
    return true;
}

/**
 * @brief Checks whether contactless payments are enabled.
 * @return true if enabled, false otherwise.
 */
bool CreditCard::isContactlessEnabled() const
{
    return contactless_enable_;
}

/**
 * @brief Enables or disables contactless payments.
 * @param enable true to enable, false to disable.
 */
void CreditCard::enableContactless(bool enable)
{
    contactless_enable_ = enable;
}
