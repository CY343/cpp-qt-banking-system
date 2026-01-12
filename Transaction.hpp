#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include <string>
#include <ctime>

/**
 * @class Transaction
 * @brief Represents a single account transaction (deposit, withdrawal, fee, etc.).
 *
 * A Transaction stores:
 * - transaction type (deposit/withdrawal/fee/etc.)
 * - transaction amount
 * - account balance after the transaction
 * - optional description/note
 * - timestamp (time_t)
 * - optional account number (useful when exporting logs)
 *
 * @note Timestamps are stored as time_t for compatibility with localtime_s (MSVC).
 */
class Transaction {
public:
    /**
     * @enum Type
     * @brief Categorizes transactions by purpose.
     */
    enum Type { DEPOSIT, WITHDRAWAL, INTEREST, FEE, PURCHASE, UNKNOWN };

    // --------------------
    // Constructors
    // --------------------

    /**
     * @brief Default constructor.
     *
     * Creates a placeholder transaction:
     * - accountNumber = "None"
     * - type = DEPOSIT
     * - amount = 0.0
     * - balanceAfter = 0.0
     * - empty description
     * - timestamp = now
     */
    Transaction();

    /**
     * @brief Construct a transaction with an account number.
     *
     * @param accountNumber The account number associated with this transaction.
     * @param type Transaction category.
     * @param amount Transaction amount (e.g., deposited or withdrawn).
     * @param balanceAfter Account balance after the transaction is applied.
     * @param description Optional description/note (default: "").
     * @param timestamp Time of transaction (default: now).
     */
    Transaction(const std::string& accountNumber,
                Type type,
                double amount,
                double balanceAfter,
                const std::string& description = "",
                time_t timestamp = std::time(nullptr));

    /**
     * @brief Construct a transaction without an account number.
     *
     * Sets accountNumber_ to "None".
     *
     * @param type Transaction category.
     * @param amount Transaction amount.
     * @param balanceAfter Account balance after the transaction is applied.
     * @param description Optional description/note (default: "").
     * @param timestamp Time of transaction (default: now).
     */
    Transaction(Type type,
                double amount,
                double balanceAfter,
                const std::string& description = "",
                time_t timestamp = std::time(nullptr));

    // --------------------
    // Accessors
    // --------------------

    /**
     * @brief Get transaction type.
     * @return Transaction::Type enum value.
     */
    Type getType() const;

    /**
     * @brief Get transaction amount.
     * @return Amount for this transaction.
     */
    double getAmount() const;

    /**
     * @brief Get account balance after transaction.
     * @return Balance after applying this transaction.
     */
    double getBalanceAfter() const;

    /**
     * @brief Get transaction timestamp.
     * @return time_t timestamp.
     */
    time_t getTimeStamp() const;

    /**
     * @brief Get associated account number.
     * @return Account number string (or "None" if not provided).
     */
    std::string getAccountNumber() const;

    /**
     * @brief Get description/note.
     * @return Description string.
     */
    std::string getDescription() const;

    // --------------------
    // Type Conversion
    // --------------------

    /**
     * @brief Convert a transaction type to a string label.
     * @param type Transaction type.
     * @return String representation such as "DEPOSIT", "WITHDRAWAL", etc.
     */
    static std::string typeToString(Type type);

    /**
     * @brief Convert a string label to a transaction type.
     * @param str String label (e.g. "DEPOSIT").
     * @return Corresponding Transaction::Type or UNKNOWN if no match.
     */
    static Type stringToType(const std::string& str);

    // --------------------
    // Formatting / Export
    // --------------------

    /**
     * @brief Convert transaction to human-readable string.
     *
     * Example format:
     *   "2026-01-12 15:40:10 - DEPOSIT: $50.00 | Balance: $1200.00 | Note: paycheck"
     *
     * @return Human-readable summary string.
     */
    std::string toString() const;

    /**
     * @brief Convert transaction to a CSV row.
     *
     * Format:
     *   "YYYY-MM-DD HH:MM:SS","TYPE",amount,balanceAfter,"description","accountNumber"
     *
     * @return CSV row string.
     */
    std::string toCSV() const;

private:
    std::string accountNumber_; ///< Account number associated with transaction (or "None").
    Type type_;                 ///< Transaction type category.
    double amount_;             ///< Transaction amount.
    double balanceAfter_;       ///< Balance after applying the transaction.
    std::string description_;   ///< Optional description/note.
    time_t timestamp_;          ///< Timestamp stored as time_t (compatible with localtime_s).
};

#endif // TRANSACTION_HPP
