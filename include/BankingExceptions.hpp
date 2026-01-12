#ifndef BANKINGEXCEPTIONS_HPP
#define BANKINGEXCEPTIONS_HPP

#include <stdexcept>
#include <string>

// Forward declaration of SQL namespace
namespace sql {
    class Connection;
}

/**
 * @brief Base exception class for all banking-related exceptions
 */
class BankingException : public std::runtime_error {
public:
    explicit BankingException(const std::string& msg)
        : std::runtime_error(msg) {}
    
    virtual std::string errorCode() const noexcept = 0;
    virtual ~BankingException() = default;
};

/**
 * @brief Exception for database-related errors
 */
class DatabaseException : public BankingException {
public:
    enum class Operation { QUERY, CONNECTION, TRANSACTION, UNKNOWN };
    
    explicit DatabaseException(const std::string& msg, 
                              Operation op = Operation::UNKNOWN,
                              int errorCode = 0)
        : BankingException(msg),
          operation_(op),
          errorCode_(errorCode) {}
    
    std::string errorCode() const noexcept override {
        return "DB-" + std::to_string(static_cast<int>(operation_)) + 
               "-" + std::to_string(errorCode_);
    }
    
    Operation getOperation() const noexcept { return operation_; }
    int getNativeError() const noexcept { return errorCode_; }
    
private:
    Operation operation_;
    int errorCode_;
};

/**
 * @brief Exception for security-related errors
 */
class SecurityException : public BankingException {
public:
    enum class Type { AUTHENTICATION, AUTHORIZATION, ENCRYPTION, VALIDATION, UNKNOWN };
    
    explicit SecurityException(const std::string& msg, Type type = Type::UNKNOWN)
        : BankingException(msg), type_(type) {}
    
    std::string errorCode() const noexcept override {
        return "SEC-" + std::to_string(static_cast<int>(type_));
    }
    
    Type getType() const noexcept { return type_; }
    
private:
    Type type_;
};

/**
 * @brief Exception for transaction-related errors
 */
class TransactionException : public BankingException {
public:
    enum class Type { INVALID_AMOUNT, INSUFFICIENT_FUNDS, LIMIT_EXCEEDED, UNKNOWN };
    
    TransactionException(const std::string& msg, double amount, Type type = Type::UNKNOWN)
        : BankingException(msg),
          amount_(amount),
          type_(type) {}
    
    std::string errorCode() const noexcept override {
        return "TX-" + std::to_string(static_cast<int>(type_));
    }
    
    double getAmount() const noexcept { return amount_; }
    Type getType() const noexcept { return type_; }
    
private:
    double amount_;
    Type type_;
};

/**
 * @brief Exception for account-related errors
 */
class AccountException : public BankingException {
public:
    enum class Type { NOT_FOUND, CLOSED, LOW_BALANCE, INVALID_OPERATION, UNKNOWN };
    
    explicit AccountException(const std::string& msg, Type type = Type::UNKNOWN)
        : BankingException(msg), type_(type) {}
    
    std::string errorCode() const noexcept override {
        return "ACCT-" + std::to_string(static_cast<int>(type_));
    }
    
    Type getType() const noexcept { return type_; }
    
private:
    Type type_;
};

/**
 * @brief Exception for card-related errors
 */
class CardException : public BankingException {
public:
    enum class Type { EXPIRED, BLOCKED, INVALID, LIMIT_EXCEEDED, UNKNOWN };
    
    explicit CardException(const std::string& msg, Type type = Type::UNKNOWN)
        : BankingException(msg), type_(type) {}
    
    std::string errorCode() const noexcept override {
        return "CARD-" + std::to_string(static_cast<int>(type_));
    }
    
    Type getType() const noexcept { return type_; }
    
private:
    Type type_;
};

/**
 * @brief RAII class for managing database transactions
 */
class TransactionGuard {
public:
    /**
     * @brief Construct a new Transaction Guard object
     * 
     * @param conn Database connection to manage
     */
    explicit TransactionGuard(void* conn);
    
    /**
     * @brief Commit the transaction
     */
    void commit();
    
    /**
     * @brief Destroy the Transaction Guard object
     *        Automatically rolls back if not committed
     */
    ~TransactionGuard();
    
    // Disable copying
    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;
    
private:
    void* conn_;  // Opaque pointer to sql::Connection
    bool committed_ = false;
};

#endif // BANKINGEXCEPTIONS_HPP