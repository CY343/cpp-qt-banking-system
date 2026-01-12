#ifndef CARD_HPP
#define CARD_HPP

#include "CardGenerator.hpp"
#include "SecurityHelper.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <ctime>
#include <openssl/rand.h>

/**
 * @class Card
 * @brief Abstract base class representing a generic bank card.
 *
 * The Card class stores common card data (number, expiration date, CVV)
 * and provides shared logic such as activation state and expiration checks.
 *
 * Derived classes (e.g., CreditCard, DebitCard) must implement:
 * - processPayment()
 * - checkIfExpired()
 */
class Card {
private:
    std::string card_number_;  ///< Card number (typically 16 digits)
    std::string expiration_;   ///< Expiration date in "MM/YY" format
    std::string cvv_;          ///< Card CVV as a 3-digit string
    bool isActivated_;         ///< True if card is activated
    bool isExpired_;           ///< True if card is marked expired (manual/internal flag)

protected:
    /**
     * @brief Generate a future expiry date in "MM/YY" format.
     *
     * Uses the current date and adds @p validYears to the year.
     * The month is randomly generated.
     *
     * @param validYears Number of years the card should remain valid (default: 3).
     * @return Expiry date string formatted as "MM/YY".
     */
    static std::string generateExpiryDate(int validYears = 3);

    /**
     * @brief Generate a random 3-digit CVV code.
     *
     * Uses OpenSSL RAND_bytes to generate secure random bytes,
     * then maps them into a 3-digit range.
     *
     * @return A 3-digit CVV string.
     * @throws std::runtime_error if secure random generation fails.
     */
    static std::string generateCVV();

    /**
     * @brief Protected constructor for use by derived card classes.
     *
     * Initializes activation and expiration flags to false.
     * If @p auto_generate is true, it generates card number, expiry, and CVV.
     *
     * @param bin_prefix Bank Identification Number (BIN) prefix used for generation.
     * @param auto_generate If true, generate card_number_, expiration_, and cvv_.
     */
    Card(const std::string& bin_prefix, bool auto_generate)
        : isActivated_(false), isExpired_(false)
    {
        if (auto_generate) {
            card_number_ = CardGenerator::generate(bin_prefix);
            expiration_  = generateExpiryDate();
            cvv_         = generateCVV();
        }
    }

    /**
     * @brief Get the expiration date string (MM/YY).
     * @return Expiration date.
     */
    std::string getExpiration() const;

    /**
     * @brief Get the CVV string.
     * @return CVV.
     */
    std::string getCvv() const;

public:
    /**
     * @brief Default constructor.
     *
     * Initializes the card fields to placeholder values
     * and sets activation/expiration flags to false.
     */
    Card();

    /**
     * @brief Construct a Card with explicit activation and expiration state.
     *
     * If @p isExpired is false, the expiration is set to a valid future date.
     * If @p isExpired is true, expiration is set to "00/00".
     *
     * @param isActivated Whether card starts activated.
     * @param isExpired Whether card starts expired.
     */
    Card(const bool isActivated, const bool isExpired);

    /**
     * @brief Check whether the card is activated.
     * @return True if activated, false otherwise.
     */
    bool isActivated() const;

    /**
     * @brief Set activation state.
     * @param isActivate True to activate, false to deactivate.
     */
    void setActivated(bool isActivate);

    /**
     * @brief Process a payment using this card.
     *
     * Must be implemented by derived classes because payment logic
     * depends on card type/account rules.
     *
     * @param amount Amount to charge.
     * @return True if processed successfully, false otherwise.
     */
    virtual bool processPayment(double amount) = 0;

    /**
     * @brief Validate card fields.
     *
     * Base validation checks that card number, expiration and CVV exist
     * and are not placeholder values.
     *
     * @return True if card data appears valid, false otherwise.
     */
    virtual bool validate() const;

    /**
     * @brief Get the full card number (unmasked).
     * @return Card number string.
     */
    std::string getCardNumber() const;

    /**
     * @brief Determine if the card is expired based on the expiration date and current date.
     *
     * This checks expiration_ (MM/YY) against today's month/year.
     * Derived classes may override this if they enforce additional rules.
     *
     * @return True if expired, false otherwise.
     */
    virtual bool isExpired() const;

    /**
     * @brief Derived-class-specific expiration rule check.
     *
     * Some card types may choose to interpret expiration differently
     * or sync with account/card status logic.
     *
     * @return True if expired, false otherwise.
     */
    virtual bool checkIfExpired() = 0;

    /**
     * @brief Mark the card as expired and deactivate it.
     *
     * Sets expiration to "00/00", sets internal expired flag,
     * and deactivates the card.
     */
    void markExpired();

    // ----------------------------
    // Testing helpers (GTest only)
    // ----------------------------

    /// @brief Force-set card number (testing only).
    void setCardNumber(const std::string& number);

    /// @brief Force-set expiration date (testing only).
    void setExpiration(const std::string& expiration);

    /// @brief Force-set CVV (testing only).
    void setCvv(const std::string& cvv);

    /**
     * @brief Generate a random 16-digit number beginning with @p prefix.
     * @param prefix First digit(s) of the card number (default: "5").
     * @return Random card number string.
     */
    std::string generateCardNumber(const std::string& prefix = "5");

    /**
     * @brief Return masked CVV for display/logging.
     * @return Masked CVV (default "***").
     */
    virtual std::string getMaskedCvv() const;

    /**
     * @brief Return masked expiration for display/logging.
     * @return Masked expiration (default "***").
     */
    virtual std::string getDisplayExpiration() const;
};

#endif
