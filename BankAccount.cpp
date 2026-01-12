/**
 * @file BankAccount.cpp
 * @brief Implements the BankAccount base class.
 *
 * Features:
 * - Auto-generates unique account numbers via a static counter
 * - Tracks balance and transaction history
 * - Detects low-balance transitions and triggers a callback hook (onLowBalance)
 */
#include "BankAccount.hpp"
#include "CreditCard.hpp"
#include <iostream>
#include <string>

// A static integer to generate unique account numbers for each bank account
// Starts at 1000 and increments with each new account created
int BankAccount::next_account_number_ = 1000;

/**
 * @brief Sets the account balance (internal use only).
 *
 * Intended for derived classes (e.g., CheckingAccount) that need to
 * adjust balance as part of specialized withdrawal rules.
 *
 * @param newBalance New balance value.
 */
void BankAccount::setBalanceInternal(double newBalance)
{
    account_balance_ = newBalance;
}

//************************************Constructors************************************
/**
 * @brief Constructs a BankAccount with default values and a generated account number.
 *
 * Initializes:
 * - deposit_ / withdrawal_: 0.0
 * - account_balance_: 0.0
 * - interest_rate_: 0.0
 * - accountType_: "NONE"
 * - account_number_: auto-generated using next_account_number_
 * - hasLowBalance_: false (updated by setLowBalance() only when called)
 */
BankAccount::BankAccount():deposit_(0.0),
    withdrawal_(0.0),
    account_number_(next_account_number_++),
    hasLowBalance_(false),
    accountType_("NONE"),
    account_balance_(0.0),
    interest_rate_(0.0){}

/**
 * @brief Constructs a BankAccount with provided initial values and a generated account number.
 *
 * Assigns a unique account number by incrementing next_account_number_.
 * Calls setLowBalance() to set hasLowBalance_ and trigger onLowBalance() if needed.
 *
 * @param deposit Initial deposit amount stored in deposit_.
 * @param withdrawal Initial withdrawal amount stored in withdrawal_.
 * @param account_balance Starting balance.
 * @param interest_rate Starting interest rate.
 * @param accountType Account type label (e.g., "CHECKING", "SAVINGS").
 */
BankAccount::BankAccount(const double &deposit,
                         const double &withdrawal,
                         const double &account_balance,
                         const double &interest_rate,
                         const std::string& accountType)
    :
    deposit_(deposit),
    withdrawal_(withdrawal),
    account_number_(next_account_number_++),
    hasLowBalance_(false),
    accountType_(accountType),
    account_balance_(account_balance),
    interest_rate_(interest_rate)
    {setLowBalance();}

/**
 * @brief Constructs a BankAccount using an existing account number (restore/load use-case).
 *
 * @param exisiting_account_number Account number to assign.
 * @param balance Starting balance.
 * @param accountType Account type label.
 *
 * @note Calls setLowBalance() to update low-balance state for the restored balance.
 */
BankAccount::BankAccount(int exisiting_account_number, double balance, const std::string &accountType)
    : deposit_(0.0),
    withdrawal_(0.0),
    account_number_(exisiting_account_number),
    hasLowBalance_(false),
    accountType_(accountType),
    account_balance_(balance),
    interest_rate_(0.0)
{
    setLowBalance();
}

/**
 * @brief Retrieves the account balance for the bank account
 *
 * @return account_balance_ as a double
 */
double BankAccount::getAccountBalance() const
{
    return account_balance_;
}

/**
 * @brief Deposits money into the account.
 *
 * Adds amount to account_balance_, records a DEPOSIT transaction,
 * and updates low-balance status.
 *
 * @param amount Amount to deposit (must be > 0).
 * @return true if deposit succeeds; false if amount <= 0.
 */
bool BankAccount::applyDeposit(const double &amount)
{
    if (amount <= 0) return false;
    account_balance_ += amount;
    transactions_.emplace_back(
        std::to_string(account_number_),  // Account number as string
        Transaction::DEPOSIT,
        amount,
        account_balance_,
        "Deposit",
        std::time(nullptr)
        );
    setLowBalance();
    return true;
}

/**
 * @brief Withdraws money from the account.
 *
 * Subtracts amount from account_balance_, records a WITHDRAWAL transaction,
 * and updates low-balance status.
 *
 * @param amount Amount to withdraw (must be > 0 and <= current balance).
 * @return true if withdrawal succeeds.
 *
 * @throws std::invalid_argument If amount <= 0.
 * @throws std::runtime_error If balance is insufficient.
 */
bool BankAccount::applyWithdraw(const double &amount)
{
    if (amount <= 0) throw std::invalid_argument("Withdrawal amount must be positive");
    if (account_balance_ < amount) throw std::runtime_error("Insufficient funds for withdrawal");

    account_balance_ -= amount;
    transactions_.emplace_back(
        std::to_string(account_number_),  // Account number as string
        Transaction::WITHDRAWAL,
        amount,
        account_balance_,
        "Withdrawal",
        std::time(nullptr)
        );
    setLowBalance();
    return true;
}

const std::string &BankAccount::getAccountType() const
{
    return accountType_;
}

/**
 * @brief Retrieves account number for the bank account
 *
 * @return account_number_ as a int
 */
int BankAccount::getAccountNumber() const
{

    return account_number_;
}


/**
 * @brief Checks if the account has a low balance
 *
 * This function returns whether the account balance is below the threshold (500.0)
 * It uses 'hasLowBalance_' flag to determine the status
 *
 * @return True if the account balance is below the threshold, false otherwise
 */
bool BankAccount::isLowBalance() const
{
    return hasLowBalance_;
}

/**
 * @brief Retrieves the interest rate for the bank account
 *
 * @return interest_rate_ as a double
 */
double BankAccount::getInterestRate() const
{
    return interest_rate_;
}

/**
 * @brief Sets the interest rate for the bank account
 *
 * This function allows you to set a new interest rate for the bank account
 * The interest rate can be used for calculations related to interest earned on the account balance
 * @param a const double reference interes_rate
 *
 * @return True if the interest rate was successfully updated, false otherwise
 */
bool BankAccount::setEarningInterestRate(const double &interes_rate)
{
    interest_rate_ = interes_rate;
    return true;
}

/**
 * @brief Updates low-balance state and triggers onLowBalance() on transition.
 *
 * Sets hasLowBalance_ to true if account_balance_ < LOW_BALANCE_THRESHOLD.
 * If the account was previously not low-balance and becomes low-balance,
 * calls onLowBalance().
 *
 * @note This function does not spam alerts repeatedly; it triggers only
 *       when crossing the threshold.
 */
void BankAccount::setLowBalance()
{
    const bool previous = hasLowBalance_;
    hasLowBalance_ = (account_balance_ < LOW_BALANCE_THRESHOLD);

    if(!previous && hasLowBalance_)
    {
        onLowBalance();
    }
}

/**
 * @brief Low-balance callback hook.
 *
 * Default behavior prints an alert to std::cerr. Derived classes may override
 * to implement fees, notifications, or UI events.
 */
void BankAccount::onLowBalance()
{
    std::cerr << "Low balance alert! Account #" << account_number_ << " has $ " << account_balance_ << std::endl;
}

/**
 * @brief Returns the transaction history (read-only).
 * @return Const reference to stored transactions.
 */
const std::vector<Transaction>& BankAccount::getTransactions() const {
    return transactions_;
}

/**
 * @brief Adds a transaction record to the account history.
 *
 * @param type Transaction type.
 * @param amount Transaction amount.
 * @param timestamp Timestamp of the event.
 *
 * @note Uses the current account_balance_ as the “balance after” value.
 */
void BankAccount::addTransaction(Transaction::Type type, double amount, time_t timestamp)
{
    transactions_.emplace_back(
        std::to_string(account_number_),
        type,
        amount,
        account_balance_,  // Use current balance as balanceAfter
        "",                // No description
        timestamp
        );
}
