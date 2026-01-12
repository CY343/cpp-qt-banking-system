/**
 * @file SavingAccount.hpp
 * @brief Declares the SavingAccount class.
 *
 * SavingAccount is a BankAccount type that enforces:
 * - A minimum required balance
 * - A maximum number of withdrawals per month
 * - Monthly interest accrual (recorded as a transaction)
 */

#ifndef SAVINGACCOUNT_HPP
#define SAVINGACCOUNT_HPP

#include "BankAccount.hpp"
#include <string>

/**
 * @class SavingAccount
 * @brief Savings account implementation with minimum balance and monthly withdrawal limits.
 *
 * SavingAccount extends BankAccount by adding:
 * - Minimum balance requirement (default: 500.0)
 * - Monthly withdrawal limit (default: 6 withdrawals per month)
 * - Interest rate used for monthly interest calculations
 *
 * Interest is applied monthly as:
 *   monthly_interest = balance * (interest_rate / 12)
 * and is recorded as a Transaction::INTEREST entry.
 */
class SavingAccount : public BankAccount
{
private:
    /** Minimum balance required to remain in the account after withdrawal. */
    double min_balance_;

    /** Number of withdrawals performed during the current month. */
    int withdrawal_times_this_month_;

    /**
     * @brief Savings account interest rate (e.g., 0.02 for 2% APR).
     *
     * @note This member is specific to SavingAccount and is used for monthly interest.
     */
    double interest_rate_;

public:
    //************************************Constructors************************************

    /**
     * @brief Default constructor.
     *
     * Initializes:
     * - min_balance_ = 500.0
     * - withdrawal_times_this_month_ = 0
     * - interest_rate_ = 0.0
     * - Base BankAccount constructed with its default constructor
     */
    SavingAccount();

    /**
     * @brief Constructs a SavingAccount with an initial balance and interest rate.
     *
     * Initializes:
     * - min_balance_ = 500.0
     * - withdrawal_times_this_month_ = 0
     * - interest_rate_ = interest_rate
     *
     * If initial_balance > 0, applies a deposit into the base account.
     *
     * @param initial_balance Starting balance to deposit (must be > 0 to take effect).
     * @param interest_rate Annual interest rate (e.g., 0.02 for 2% APR).
     */
    SavingAccount(double initial_balance, double interest_rate);

    /**
     * @brief Constructs a SavingAccount using an existing account number and balance.
     *
     * Intended for restoring accounts from persistent storage or testing.
     *
     * @param accountNumber Existing account number.
     * @param balance Restored balance.
     * @param customerId Identifier string passed to BankAccount (in your current design).
     *
     * @note This constructor does not generate a new account number.
     */
    SavingAccount(int accountNumber, double balance, const std::string& customerId);

    //************************************Core Operations************************************

    /**
     * @brief Withdraws money while enforcing savings constraints.
     *
     * Enforces:
     * - Remaining balance must stay >= min_balance_
     * - Monthly withdrawals must be < 6
     *
     * If checks pass, delegates the actual withdrawal to BankAccount::applyWithdraw(),
     * then increments withdrawal_times_this_month_.
     *
     * @param amount Amount to withdraw.
     * @return true if withdrawal succeeds.
     *
     * @throws std::runtime_error If minimum balance would be violated.
     * @throws std::runtime_error If monthly withdrawal limit is exceeded.
     * @throws std::invalid_argument / std::runtime_error Propagated from BankAccount::applyWithdraw()
     *         if base withdrawal fails.
     */
    bool applyWithdraw(const double& amount) override;

    //************************************Interest************************************

    /**
     * @brief Applies monthly interest and records it as a transaction.
     *
     * Calculates:
     *   interest = balance * (interest_rate_ / 12)
     * Deposits the interest and records a Transaction::INTEREST entry.
     *
     * @note This is equivalent to applyInterest() in this class; you may keep only one.
     */
    void applyMonthlyInterest();

    /**
     * @brief Applies monthly interest (override of BankAccount hook).
     *
     * Uses the same monthly interest calculation and records Transaction::INTEREST.
     */
    void applyInterest() override;

    //************************************Monthly Maintenance************************************

    /**
     * @brief Resets the monthly withdrawal counter (call at start of new month).
     */
    void resetMonthlyWithdrawals();

    //************************************Getters************************************

    /**
     * @brief Returns the minimum balance requirement.
     */
    double getMinBalance() const;

    /**
     * @brief Returns the annual interest rate for this savings account.
     */
    double getInterestRate() const;

    /**
     * @brief Returns the number of withdrawals made during the current month.
     */
    int getWithdrawalTimesThisMonth() const;
};

#endif // SAVINGACCOUNT_HPP
