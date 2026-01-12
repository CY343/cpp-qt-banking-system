/**
 * @class BankAccount
 * @brief Abstract base class for all bank account types (e.g., Checking, Savings).
 *
 * Responsibilities:
 * - Maintain account balance and account identity (account number)
 * - Record transactions (deposits/withdrawals) in a history list
 * - Track low-balance state and trigger a hook when balance becomes low
 * - Provide a polymorphic interface for withdrawals and interest application
 *
 * Design Notes:
 * - This class is abstract because applyInterest() is pure virtual.
 * - Uses shared_from_this() to allow derived classes to safely create shared_ptr
 *   references to themselves when needed (only valid if the object is owned by a shared_ptr).
 */
#ifndef BANKACCOUNT_HPP
#define BANKACCOUNT_HPP

#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include "Transaction.hpp"

/**
 * @brief Forward declarations to reduce header dependencies.
 */
class CreditCard;
class DebitCard;


class BankAccount : public std::enable_shared_from_this<BankAccount>
{
private:
    /** Unique account number assigned during construction. */
    const int account_number_;

    /** Current balance of the account. */
    double account_balance_;

    /** Interest rate applied by derived account types (e.g., savings). */
    double interest_rate_;

    /** Account type label (e.g., "CHECKING", "SAVINGS"). */
    std::string accountType_;

    /** True if balance is below LOW_BALANCE_THRESHOLD. */
    bool hasLowBalance_;

    /** Last deposit value stored (if used by your design). */
    double deposit_;

    /** Last withdrawal value stored (if used by your design). */
    double withdrawal_;

    /**
     * @brief Transaction history of the account.
     *
     * Stores a chronological record of deposits/withdrawals (and optionally
     * interest/fees if you log them).
     */
    std::vector<Transaction> transactions_;

    /** Low balance threshold used for hasLowBalance_ logic. */
    static constexpr double LOW_BALANCE_THRESHOLD = 500.0;

    /**
     * @brief Debit card associated with this account (if issued).
     *
     * @note In your design, debit cards are typically associated with checking accounts.
     * nullptr means no debit card has been issued/linked.
     */
    std::shared_ptr<DebitCard> debitCard_;

    /** Internal generator for unique account numbers. */
    static int next_account_number_;

protected:
    /**
     * @brief Hook called when the account transitions into a low-balance state.
     *
     * Derived classes may override to apply fees, warnings, notifications, etc.
     */
    virtual void onLowBalance();

    /**
     * @brief Updates hasLowBalance_ based on account_balance_ and LOW_BALANCE_THRESHOLD.
     *
     * @note Intended to be called after operations that change balance (deposit/withdraw).
     */
    void setLowBalance();

    /**
     * @brief Sets the account balance (internal use only).
     *
     * Intended for derived classes (e.g., CheckingAccount) that need to
     * adjust balance as part of specialized withdrawal rules.
     *
     * @param newBalance New balance value.
     */
    void setBalanceInternal(double newBalance);

public:
    /**
     * @brief Constructs a new bank account with an auto-generated account number.
     *
     * @note Initializes balance/rate/type to defaults determined in the implementation.
     */
    BankAccount();

    /**
     * @brief Constructs a bank account with the provided initial state.
     *
     * @param deposit Initial deposit amount (if your implementation uses it).
     * @param withdrawal Initial withdrawal amount (if your implementation uses it).
     * @param account_balance Starting account balance.
     * @param interest_rate Starting interest rate.
     * @param accountType Label describing the account type.
     *
     * @warning Consider removing deposit/withdrawal fields if they are not persistent state.
     */
    BankAccount(const double& deposit,
                const double& withdrawal,
                const double& account_balance,
                const double& interest_rate,
                const std::string& accountType);

    /**
     * @brief Constructs a bank account using an explicit account number.
     *
     * @param account_number Account number to assign.
     * @param balance Starting balance.
     * @param accountType Label describing the account type.
     *
     * @note Useful for restoring accounts from storage.
     */
    BankAccount(int account_number, double balance, const std::string& accountType);

    /**
     * @brief Returns the account type label.
     * @return Reference to account type string.
     */
    const std::string& getAccountType() const;

    /**
     * @brief Returns the unique account number.
     * @return Account number.
     */
    int getAccountNumber() const;

    /**
     * @brief Returns current account balance.
     * @return Current balance.
     */
    double getAccountBalance() const;

    /**
     * @brief Returns current interest rate.
     * @return Interest rate.
     */
    double getInterestRate() const;

    /**
     * @brief Indicates whether this account is currently considered low balance.
     * @return true if balance is below threshold; false otherwise.
     */
    bool isLowBalance() const;

    /**
     * @brief Sets the interest rate for this account.
     *
     * @param interest_rate New interest rate.
     * @return true (current implementation always succeeds).
     */
    bool setEarningInterestRate(const double& interest_rate);

    /**
     * @brief Deposits money into the account and records a transaction.
     *
     * @param amount Amount to deposit (must be positive).
     * @return true if deposit succeeded; false otherwise.
     */
    bool applyDeposit(const double& amount);

    /**
     * @brief Withdraws money from the account and records a transaction.
     *
     * Derived classes may override to enforce fees, overdraft rules, or limits.
     *
     * @param amount Amount to withdraw (must be > 0 and <= current balance).
     * @return true if withdrawal succeeded.
     *
     * @throws std::invalid_argument If amount <= 0.
     * @throws std::runtime_error If insufficient funds.
     */
    virtual bool applyWithdraw(const double& amount);

    /**
     * @brief Adds a transaction record to the transaction history.
     *
     * @param type Transaction type (deposit/withdraw/etc).
     * @param amount Amount involved.
     * @param timestamp Time the transaction occurred.
     */
    void addTransaction(Transaction::Type type, double amount, time_t timestamp);

    /**
     * @brief Applies interest according to the account rules.
     *
     * Pure virtual: each derived type must define how interest works.
     */
    virtual void applyInterest() = 0;

    /**
     * @brief Returns the transaction history (read-only).
     * @return Const reference to stored transactions.
     */
    const std::vector<Transaction>& getTransactions() const;
};

#endif
