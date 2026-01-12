#include "Transaction.hpp"

#include <iomanip>
#include <sstream>

// -------------------- Constructors --------------------

/**
 * @brief Default constructor.
 */
Transaction::Transaction()
    : accountNumber_("None"),
      type_(DEPOSIT),
      amount_(0.0),
      balanceAfter_(0.0),
      description_(""),
      timestamp_(std::time(nullptr))
{}

/**
 * @brief Construct a transaction with an account number.
 */
Transaction::Transaction(const std::string& accountNumber,
                         Type type,
                         double amount,
                         double balanceAfter,
                         const std::string& description,
                         time_t timestamp)
    : accountNumber_(accountNumber),
      type_(type),
      amount_(amount),
      balanceAfter_(balanceAfter),
      description_(description),
      timestamp_(timestamp)
{}

/**
 * @brief Construct a transaction without an account number.
 *
 * Sets accountNumber_ to "None".
 */
Transaction::Transaction(Type type,
                         double amount,
                         double balanceAfter,
                         const std::string& description,
                         time_t timestamp)
    : accountNumber_("None"),
      type_(type),
      amount_(amount),
      balanceAfter_(balanceAfter),
      description_(description),
      timestamp_(timestamp)
{}

// -------------------- Accessors --------------------

Transaction::Type Transaction::getType() const { return type_; }
double Transaction::getAmount() const { return amount_; }
double Transaction::getBalanceAfter() const { return balanceAfter_; }
time_t Transaction::getTimeStamp() const { return timestamp_; }
std::string Transaction::getAccountNumber() const { return accountNumber_; }
std::string Transaction::getDescription() const { return description_; }

// -------------------- Type Conversion --------------------

/**
 * @brief Convert transaction type to string label.
 */
std::string Transaction::typeToString(Type type)
{
    switch (type) {
    case DEPOSIT:    return "DEPOSIT";
    case WITHDRAWAL: return "WITHDRAWAL";
    case INTEREST:   return "INTEREST";
    case FEE:        return "FEE";
    case PURCHASE:   return "PURCHASE";
    default:         return "UNKNOWN";
    }
}

/**
 * @brief Convert string label to transaction type.
 */
Transaction::Type Transaction::stringToType(const std::string& str)
{
    if (str == "DEPOSIT")    return DEPOSIT;
    if (str == "WITHDRAWAL") return WITHDRAWAL;
    if (str == "INTEREST")   return INTEREST;
    if (str == "FEE")        return FEE;
    if (str == "PURCHASE")   return PURCHASE;
    return UNKNOWN;
}

// -------------------- String Representations --------------------

/**
 * @brief Convert transaction to a readable string.
 */
std::string Transaction::toString() const
{
    std::tm tm{};
    localtime_s(&tm, &timestamp_); // MSVC-safe

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);

    std::ostringstream oss;
    oss << buffer << " - "
        << typeToString(type_) << ": $"
        << std::fixed << std::setprecision(2) << amount_
        << " | Balance: $" << std::fixed << std::setprecision(2) << balanceAfter_;

    if (!description_.empty()) {
        oss << " | Note: " << description_;
    }

    return oss.str();
}

// -------------------- CSV Export --------------------

/**
 * @brief Convert transaction to CSV format.
 */
std::string Transaction::toCSV() const
{
    std::tm tm{};
    localtime_s(&tm, &timestamp_);

    char dateBuffer[32];
    std::strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d %H:%M:%S", &tm);

    std::ostringstream oss;
    oss << "\"" << dateBuffer << "\","
        << "\"" << typeToString(type_) << "\","
        << std::fixed << std::setprecision(2) << amount_ << ","
        << std::fixed << std::setprecision(2) << balanceAfter_ << ","
        << "\"" << description_ << "\","
        << "\"" << accountNumber_ << "\"";

    return oss.str();
}
