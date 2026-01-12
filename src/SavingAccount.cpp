/**
 * @file SavingAccount.cpp
 * @brief Implements the SavingAccount class.
 *
 * Implements savings-specific rules:
 * - Minimum balance enforcement
 * - Monthly withdrawal limit
 * - Monthly interest accrual and transaction logging
 */

#include "SavingAccount.hpp"
#include <stdexcept>
#include <ctime>

//************************************Constructors************************************

/**
 * @brief Default constructor.
 */
SavingAccount::SavingAccount()
    : BankAccount(),
      min_balance_(500.0),
      withdrawal_times_this_month_(0),
      interest_rate_(0.0)
{}

/**
 * @brief Constructs savings account with initial balance and interest rate.
 */
SavingAccount::SavingAccount(double initial_balance, double interest_rate)
    : BankAccount(),
      min_balance_(500.0),
      withdrawal_times_this_month_(0),
      interest_rate_(interest_rate)
{
    if (initial_balance > 0) {
        applyDeposit(initial_balance);
    }
}

/**
 * @brief Constructs savings account for restore/load use-case.
 */
SavingAccount::SavingAccount(int accountNumber, double balance, const std::string& customerId)
    : BankAccount(accountNumber, balance, customerId),
      min_balance_(500.0),
      withdrawal_times_this_month_(0),
      interest_rate_(0.0)
{}

//************************************Core Operations************************************

/**
 * @brief Withdraw with minimum balance and monthly limit rules.
 */
bool SavingAccount::applyWithdraw(const double& amount)
{
    if ((getAccountBalance() - amount) < min_balance_) {
        throw std::runtime_error("Withdrawal violates minimum balance requirements");
    }

    if (withdrawal_times_this_month_ >= 6) {
        throw std::runtime_error("Maximum monthly withdrawal exceeded.");
    }

    // Base class may throw if amount <= 0 or insufficient funds.
    if (BankAccount::applyWithdraw(amount)) {
        withdrawal_times_this_month_++;
        return true;
    }

    return false;
}

//************************************Interest************************************

/**
 * @brief Applies monthly interest to the account and records a transaction.
 */
void SavingAccount::applyMonthlyInterest()
{
    const double interest = getAccountBalance() * (interest_rate_ / 12.0);
    applyDeposit(interest);
    addTransaction(Transaction::INTEREST, interest, std::time(nullptr));
}

/**
 * @brief Applies monthly interest (BankAccount hook).
 */
void SavingAccount::applyInterest()
{
    const double monthly_interest = getAccountBalance() * (getInterestRate() / 12.0);
    applyDeposit(monthly_interest);
    addTransaction(Transaction::INTEREST, monthly_interest, std::time(nullptr));
}

//************************************Monthly Maintenance************************************

/**
 * @brief Resets monthly withdrawal counter.
 */
void SavingAccount::resetMonthlyWithdrawals()
{
    withdrawal_times_this_month_ = 0;
}

//************************************Getters************************************

double SavingAccount::getMinBalance() const
{
    return min_balance_;
}

double SavingAccount::getInterestRate() const
{
    return interest_rate_;
}

int SavingAccount::getWithdrawalTimesThisMonth() const
{
    return withdrawal_times_this_month_;
}
