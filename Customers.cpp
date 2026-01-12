/**
 * @file Customers.cpp
 * @brief Implements the Customers class.
 *
 * Responsibilities:
 * - Generate and store unique customer IDs (UUID)
 * - Validate and normalize customer profile fields (name/email/phone/address)
 * - Manage ownership links to bank accounts and cards
 */
#include"Customers.hpp"
#include<iostream>
#include<string>
#include <utility>

// UUID library
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

//************************************helper functions************************************
/**
 * @brief A helper function to generate customer id.
 *
 * @return A string (unique customer id)
 */
std::string generate_uuid()
{
    boost::uuids::random_generator generator;
    return boost::uuids::to_string(generator());
}

//************************************Constructors************************************
/**
 * @brief Creates a customer profile with default/empty fields and a UUID.
 *
 * Initializes:
 * - Name: ""
 * - Email: ""
 * - Phone: ""
 * - Address: ""
 * - Age: 0
 * - ID: randomly generated UUID
 *
 * @note Fields remain empty until set via setters.
 */

Customers::Customers(): customers_name_(""),
    customers_email_(""),
    customers_phone_number_(""),
    customers_id_(generate_uuid()),
    customers_age_(0),
    customers_address_(""){}


/**
 * @brief Creates a customer profile and attempts to validate all input fields.
 *
 * Initializes fields by calling validation setters in this order:
 * 1. setName(name)        (uppercases valid names)
 * 2. setEmail(email)      (uppercases valid emails)
 * 3. setPhoneNumber(phone_number) (stores 10 digits only)
 * 4. setAge(age)          (requires age >= 0)
 * 5. setAddress(address)  (uppercases valid address)
 *
 * @param name Alphabetic+space characters only.
 * @param email Must match the system email regex pattern.
 * @param phone_number Must contain 10 digits after removing non-digits.
 * @param age Must be non-negative.
 * @param address Must be non-empty (not all whitespace).
 *
 * @note If any setter fails validation, that field remains at its default value.
 * @warning Name, email, and address are stored in UPPERCASE when valid.
 *
 * @code
 * Customers c("John Doe", "john@doe.com", "(123)456-7890", 30, "123 Main St");
 * // Stored as: NAME="JOHN DOE", EMAIL="JOHN@DOE.COM", PHONE="1234567890"
 * @endcode
 */
Customers::Customers(const std::string& name,
                     const std::string& email,
                     const std::string& phone_number,
                     int age,
                     const std::string& address):
    customers_name_(""),
    customers_email_(""),
    customers_phone_number_(""),
    customers_id_(generate_uuid()),
    customers_age_(0),
    customers_address_("")

{
    setName(name);
    setEmail(email);
    setPhoneNumber(phone_number);
    setAge(age);
    setAddress(address);

}

/**
 * @brief Retrieves the customer's name.
 *
 * @return The name of the customer as a std::string.
 */
std::string Customers::getName()const
{
    return customers_name_;
}


/**
 * @brief Sets the customer's name after validating and formatting it.
 *
 * Validates that the name contains only alphabetic characters and spaces.
 * Converts all alphabetic characters to uppercase before setting.
 *
 * @param name The new name to set.
 * @return true if the name is valid and set successfully, false otherwise.
 */
bool Customers::setName(const std::string& name)
{
    bool has_alpha = false;

    if(name.empty())
    {
        return false;
    }
    for(char c : name)
    {
        if(!isalpha(c) && c != ' ')
        {
            return false;
        }
        if(isalpha(c))
        {
            has_alpha = true;
        }
    }

    if(!has_alpha)
    {
        return false;
    }
    customers_name_ = name;

    for(char& c : customers_name_)
    {
        if(isalpha(c))
        {
            c = toupper(c);
        }
    }

    return true;
}


/**
 * @brief Retrieves the customer's email address.
 *
 * @return The email address of the customer as a std::string.
 */
std::string Customers::getEmail()const
{
    return customers_email_;
}


/**
 * @brief Sets the customer's email after validating its format.
 *
 * Uses a regular expression to verify that the email is in a valid format.
 * If valid, sets the customer's email; otherwise, returns false.
 *
 * @param email The email address to set.
 * @return true if the email format is valid and set successfully, false otherwise.
 */
bool Customers::setEmail(const std::string& email)
{
    std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if(!std::regex_match(email, pattern))
    {
        return false;
    }
    std::string upper_email = email;
    std::transform(upper_email.begin(), upper_email.end(), upper_email.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    customers_email_ = upper_email;
    return true;
}


/**
 * @brief Retrieves the customer's phone number.
 *
 * @return The phone number of the customer as a std::string.
 */
std::string Customers::getPhoneNumber()const
{
    return customers_phone_number_;
}


/**
 * @brief Sets the customer's phone number after cleaning and validating it.
 *
 * Removes all non-digit characters from the input and checks that
 * the resulting number contains exactly 10 digits.
 *
 * @param phone_number The raw phone number string input.
 * @return true if the phone number is valid and set successfully, false otherwise.
 */
bool Customers::setPhoneNumber(const std::string& phone_number)
{
    std::string clean_number;
    /// Remove all non-digit characters first
    for(char c : phone_number) {
        if(isdigit(c)) clean_number += c;
    }

    /// Then validate length
    if(clean_number.length() != 10) {
        std::cerr << "Invalid phone number length\n";
        return false;
    }

    customers_phone_number_ = clean_number;
    return true;
}


/**
 * @brief Retrieves the customer's age.
 *
 * @return The age of the customer as an integer.
 */
int Customers::getAge()const
{
    return customers_age_;
}


/**
 * @brief Sets the customer's age.
 *
 * Throws an exception if the age is negative.
 *
 * @param age The age to set.
 * @return true if the age is valid and set successfully.
 * @throws std::invalid_argument if the age is negative.
 */
bool Customers::setAge(const int age)
{
    if(age < 0)
    {
        return false;
    }
    customers_age_ = age;
    return true;
}

/**
 * @brief Links a bank account to the customer.
 *
 * Adds the given shared pointer to a BankAccount to the customer's account list.
 *
 * @param account A shared pointer to the BankAccount to link.
 */
void Customers::linkAccount(std::shared_ptr<BankAccount> account)
{
    account_.push_back(account);
}

/**
 * @brief Retrieves the list of bank accounts linked to the customer.
 *
 * @return A constant reference to a vector of shared pointers to BankAccount objects.
 */
const std::vector<std::shared_ptr<BankAccount>> &Customers::getAccount()const
{
    return account_;
}

/**
 * @brief Sets the customer's credit card collection using move semantics.
 *
 * Transfers ownership of the provided vector into the customer without copying.
 *
 * @param card Rvalue reference to a vector of shared pointers to CreditCard.
 *
 * @note After the move, the input vector is left in a valid but unspecified state.
 */
void Customers::setCreditCard(std::vector<std::shared_ptr<CreditCard>> &&card)
{
    creditCard_ = std::move(card);
}

/**
 * @brief Checks if the customer has any credit cards linked.
 *
 * @return true if the customer has one or more credit cards, false otherwise.
 */
bool Customers::hasCreditCard() const
{
    return !creditCard_.empty();
}

/**
 * @brief Retrieves all linked SavingAccount objects.
 *
 * Iterates through all linked BankAccount objects, filters those that are
 * SavingAccount instances using dynamic_pointer_cast, and returns them in a vector.
 *
 * @return A vector of shared pointers to SavingAccount objects.
 */
std::vector<std::shared_ptr<SavingAccount>> Customers::getSavingAccount() const
{
    std::vector<std::shared_ptr<SavingAccount>> result;
    for(const auto& acc : account_)
    {
        if(auto sa = std::dynamic_pointer_cast<SavingAccount>(acc))
        {
            result.push_back(sa);
        }
    }
    return result;
}

/**
 * @brief Retrieves all linked CheckingAccount objects.
 *
 * Iterates through all linked BankAccount objects, filters those that are
 * CheckingAccount instances using dynamic_pointer_cast, and returns them in a vector.
 *
 * @return A vector of shared pointers to CheckingAccount objects.
 */
std::vector<std::shared_ptr<CheckingAccount>> Customers::getCheckingAccount() const
{
    std::vector<std::shared_ptr<CheckingAccount>> result;
    for(const auto& acc : account_)
    {
        if(auto ca = std::dynamic_pointer_cast<CheckingAccount>(acc))
        {
            result.push_back(ca);
        }
    }
    return result;
}

/**
 * @brief Removes an account by account number.
 *
 * Searches the linked account list and removes any account whose
 * account number matches the provided value.
 *
 * @param account_number Account number to remove.
 * @return true if at least one account was removed, false otherwise.
 */
bool Customers::removeAccount(int account_number) {
    // Use remove-erase idiom
    auto new_end = std::remove_if(account_.begin(), account_.end(),
                                  [account_number](const std::shared_ptr<BankAccount>& acc) {
                                      return acc->getAccountNumber() == account_number;
                                  }
                                  );

    if (new_end != account_.end()) {
        account_.erase(new_end, account_.end());
        return true;
    }
    return false;
}

bool Customers::setAddress(const std::string& address) {
    if (address.empty() || std::all_of(address.begin(), address.end(), [](unsigned char c){ return std::isspace(c); })) {
        return false;
    }
    std::string upper_address = address;
    std::transform(upper_address.begin(), upper_address.end(), upper_address.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    customers_address_ = upper_address;
    return true;
}


/* 
* @brief Retrieves the customer's address.
*
* @return The address of the customer as a std::string.
*/
std::string Customers::getAddress() const
{
    return customers_address_;
}

/**
 * @brief Adds a BankAccount to the customer's account list
 * 
 * @param account A shared pointer to the BankAccount to add
 */
void Customers::addAccount(std::shared_ptr<BankAccount> account) {
    if(account) account_.push_back(account);
}

/**
 * @brief Retrieve customer id (using inline declaration)
 * 
 * @return A string Id randomly generated
 */
std::string Customers::getCustomerId()const{
    return customers_id_;
}

/**
 * @brief Retrieves the first 8 characters of the customer's ID
 * 
 * @return A string representing the short customer ID
 */
std::string Customers::getShortCustomerId() const {
    // Returns the first 8 characters of the UUID
    return customers_id_.substr(0, 8);
}

/**
 * @brief Collects all cards linked through the customer's checking accounts.
 *
 * Iterates through linked accounts, finds CheckingAccount instances, then
 * appends their debit and credit cards into a single list.
 *
 * @return Vector of Card pointers (debit + credit) collected from checking accounts.
 *
 * @note This does not read from Customers::creditCard_.
 */
std::vector<std::shared_ptr<Card>> Customers::getAllCards() const {
    std::vector<std::shared_ptr<Card>> allCards;

    for (const auto& acc : account_) {
        if (auto checking = std::dynamic_pointer_cast<CheckingAccount>(acc)) {
            // Add debit cards
            const auto& debitCards = checking->getDebitCards();
            allCards.insert(allCards.end(), debitCards.begin(), debitCards.end());

            // Add credit cards
            const auto& creditCards = checking->getCreditCards();
            allCards.insert(allCards.end(), creditCards.begin(), creditCards.end());
        }
    }

    return allCards;
}

