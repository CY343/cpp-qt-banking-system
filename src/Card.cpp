#include "Card.hpp"
#include <openssl/rand.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>
#include <stdexcept>
#include <cstdio>

/**
 * @brief Default constructor for Card.
 *
 * Initializes card fields to placeholder values and sets
 * activation and expiration flags to false.
 */
Card::Card()
    : card_number_("NONE"),
      expiration_("NONE"),
      cvv_("None"),
      isActivated_(false),
      isExpired_(false)
{}

/**
 * @brief Parameterized constructor for Card.
 *
 * Initializes activation and expiration flags using the provided values.
 * If the card is not expired, a valid future expiration date is generated.
 * If expired, expiration is set to "00/00".
 *
 * Card number and CVV are set to placeholders.
 *
 * @param isActivated Initial activation state.
 * @param isExpired Initial expiration state.
 */
Card::Card(const bool isActivated, const bool isExpired)
    : isActivated_(isActivated),
      isExpired_(isExpired)
{
    if (!isExpired_) {
        expiration_ = generateExpiryDate();
    } else {
        expiration_ = "00/00";
    }

    card_number_ = "NONE";
    cvv_ = "000";
}

/**
 * @brief Generate an expiry date string in the format "MM/YY".
 *
 * Picks a random month (1-12) using OpenSSL RAND_bytes and sets the year to:
 * (current year + validYears) as the last two digits.
 *
 * @param validYears Number of years the card should be valid.
 * @return Expiration date string formatted as "MM/YY".
 */
std::string Card::generateExpiryDate(int validYears)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time);

    unsigned char month_byte{};
    RAND_bytes(&month_byte, 1);
    int month = 1 + (month_byte % 12);

    int year = (now_tm->tm_year + 1900 + validYears) % 100;

    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << month << "/"
       << std::setw(2) << std::setfill('0') << year;
    return ss.str();
}

/**
 * @brief Generate a random 3-digit CVV string.
 *
 * Uses OpenSSL RAND_bytes to produce secure randomness and maps it into [100, 999].
 *
 * @return CVV string (3 digits).
 * @throws std::runtime_error if RAND_bytes fails.
 */
std::string Card::generateCVV()
{
    unsigned char cvv_bytes[3];
    if (RAND_bytes(cvv_bytes, sizeof(cvv_bytes)) != 1) {
        throw std::runtime_error("Failed to generate secure CVV");
    }
    return std::to_string(100 + (cvv_bytes[0] % 900));
}

/**
 * @brief Check whether the card is activated.
 * @return True if activated.
 */
bool Card::isActivated() const
{
    return isActivated_;
}

/**
 * @brief Set card activation state.
 * @param isActivate True to activate, false to deactivate.
 */
void Card::setActivated(bool isActivate)
{
    isActivated_ = isActivate;
}

/**
 * @brief Validate whether the card has required fields populated.
 *
 * Checks that card number, expiration date, and CVV are not empty
 * and not placeholder values.
 *
 * @return True if card data appears valid.
 */
bool Card::validate() const
{
    if (card_number_.empty() || card_number_ == "NONE") return false;
    if (expiration_.empty()  || expiration_  == "NONE") return false;
    if (cvv_.empty()         || cvv_         == "None") return false;
    return true;
}

/**
 * @brief Get full card number.
 * @return Card number string.
 */
std::string Card::getCardNumber() const
{
    return card_number_;
}

/**
 * @brief Check expiration using the stored expiration date and current date.
 *
 * Parses expiration_ as "MM/YY", converts YY to YYYY by adding 2000, and compares
 * to the current month/year. The card is expired if:
 * - expiration year < current year, or
 * - same year but expiration month < current month.
 *
 * @return True if expired, false otherwise.
 */
bool Card::isExpired() const
{
    std::time_t t = std::time(nullptr);
    std::tm* now = std::localtime(&t);

    int expiredMonth = 0;
    int expiredYear  = 0;

    std::sscanf(expiration_.c_str(), "%2d/%2d", &expiredMonth, &expiredYear);
    expiredYear += 2000;

    const int currentYear  = now->tm_year + 1900;
    const int currentMonth = now->tm_mon + 1;

    if (expiredYear < currentYear) return true;
    if (expiredYear == currentYear && expiredMonth < currentMonth) return true;

    return false;
}

/**
 * @brief Mark the card expired and deactivate it.
 *
 * Sets expiration_ to "00/00", sets isExpired_ to true,
 * and deactivates the card.
 */
void Card::markExpired()
{
    expiration_ = "00/00";
    isExpired_ = true;
    setActivated(false);
}

/**
 * @brief Force-set card number (testing only).
 */
void Card::setCardNumber(const std::string& number)
{
    card_number_ = number;
}

/**
 * @brief Force-set expiration (testing only).
 */
void Card::setExpiration(const std::string& expiration)
{
    expiration_ = expiration;
}

/**
 * @brief Force-set CVV (testing only).
 */
void Card::setCvv(const std::string& cvv)
{
    cvv_ = cvv;
}

/**
 * @brief Get expiration string.
 * @return Expiration date "MM/YY".
 */
std::string Card::getExpiration() const
{
    return expiration_;
}

/**
 * @brief Get CVV string.
 * @return CVV.
 */
std::string Card::getCvv() const
{
    return cvv_;
}

/**
 * @brief Generate a pseudo-random 16-digit card number beginning with prefix.
 *
 * This is intended for testing/demo. Production should use your CardGenerator.
 *
 * @param prefix Starting digit(s).
 * @return Card number string (16 digits total).
 */
std::string Card::generateCardNumber(const std::string& prefix)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 9);

    std::ostringstream oss;
    oss << prefix;
    for (int i = 1; i < 16; ++i) {
        oss << dist(gen);
    }
    return oss.str();
}

/**
 * @brief Return masked CVV for safe display.
 * @return "***"
 */
std::string Card::getMaskedCvv() const
{
    return "***";
}

/**
 * @brief Return masked expiration for safe display.
 * @return "***"
 */
std::string Card::getDisplayExpiration() const
{
    return "***";
}
