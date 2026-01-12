
/**
 * @file Customers.hpp
 * @brief Defines the Customers class used to represent banking customers.
 *
 * This file declares the Customers class, which stores customer identity,
 * validation logic, and ownership of bank accounts and cards.
 */
#ifndef CUSTOMERS_HPP
#define CUSTOMERS_HPP
#include<iostream>
#include<string>
#include<memory>
#include<vector>
#include<regex>
#include"SavingAccount.hpp"
#include"CheckingAccount.hpp"
#include"Card.hpp"

/**
 * @brief Generates a unique random customer ID using UUID.
 *
 * This ID uniquely identifies a customer across the banking system
 * and is generated automatically during construction.
 */
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

// forward declaration
class BankAccount;
class CreditCard;

/**
 * @class Customers
 * @brief Represents a customer in the banking system.
 *
 * A Customers object stores personal information such as name,
 * email, phone number, age, and address, and maintains ownership
 * of bank accounts and cards.
 */
class Customers{
    // ******************************************* Attributes *******************************************
    private:
        std::string customers_name_; // A string corresponding to the name of the customer
        std::string customers_email_; // A string corresponding to the email of the customer
        std::string customers_phone_number_; // A string corresponding to the phone number of the customer
        std::string customers_id_; // A string corresponding to the id of the customer
        int customers_age_; // An integer corresponding to the age of the customer
        std::string customers_address_; // A string corresponding to the address of the customer
        std::vector<int> accounts_; // A vector of integers corresponding to the account numbers owned by the customer

        /**
         * @brief A vector of shared pointer to BankAccount objects
         * 
         * customers can hold any types of accounts (saving or checking, both)
         */
        std::vector<std::shared_ptr<BankAccount>> account_; 

        /**
         * @brief A vector of shared pointer to CreditCard object
         * 
         * Customers can have multiple credit cards
         */
        std::vector<std::shared_ptr<CreditCard>> creditCard_;

    

    // ******************************************* Public Methods *******************************************
    public:
        /**
         * @brief Default constructor for Customer class
         * 
         * Initializes a customer without any specific details
         */
        Customers();

        /**
         * @brief Parameterized constructor for Customer class 
         * with parameters to initialize a customer object
         * 
         * This constructor allows for the creation of a customer 
         * object with a name, email, phone number, and age
         * 
         *
         * @param a const string reference name 
         * @param a const string reference email 
         * @param a const string reference phone_number 
         * @param a int age 
         */
        Customers(const std::string& name, 
                  const std::string& email, 
                  const std::string& phone_number, 
                  int age,
                  const std::string& address);

        /**
         * @brief Retrieves the name of the customer in uppercase format
         * 
         * @return std::string  uppercase name containing only alphabetic characters and spaces
         * @note Format enforced during construction/setName()
         * @see Customers::setName() for validation rules
         */
        std::string getName() const;

        /**
         * @brief Retrieves RFC 5322-compliant email address
         * 
         * @return std::string Valid email format or "NONE" if invalid
         * @warning Returned value may differ from input due to validation
         * @remark Example: "JOHN.DOE@DOMAIN.COM"
         */
        std::string getEmail()const;

        /**
         * @brief Provides the customer's age verification
         * 
         * @return int current age in years (>=0)
         * @throws std::logic_error If age was set improperly
         */
        int getAge() const;

        /**
         * @brief provides the customer's phone number 
         * 
         * @return A string represting customer phone number 
         */
        std::string getPhoneNumber()const;

        /**
         * @brief Access linked financial accounts
         * 
         * @return const std::vector<std::shared_ptr<BankAccount>>& read-only reference to account list
         * @warning Do not modify directly - use linkAccount() for additions
         * @note Empty vector indicates no linked accounts
         * @see Customers::linkAccount() for safe modification
         */
        const std::vector<std::shared_ptr<BankAccount>>& getAccount() const;

        /**
         * @brief Sets the customer's name
         * 
         * @param A const string reference name 
         * @return true if the name was successfully set
         */
        bool setName(const std::string& name);

        /**
         * @brief Sets the customer's email address
         * 
         * @param a const string reference the new email address
         * 
         * @return true if the email was successfully set
         *  */
        bool setEmail(const std::string& email);

        /**
         * @brief Sets the customer's credit card collection using move semantics.
         *
         * Transfers ownership of the provided credit card vector into the customer
         * without copying its elements.
         *
         * @param card Rvalue reference to a vector of shared pointers to CreditCard
         *             objects. The contents are moved into the customer.
         *
         * @note After the call, the passed-in vector is left in a valid but unspecified state.
         */
        void setCreditCard(std::vector<std::shared_ptr<CreditCard>>&& card);

        /**
         * @brief Sets the customer's phone number
         * 
         * @param a const string reference phone_number 
         * @return true if phone number was successfully set
         */
        bool setPhoneNumber(const std::string& phone_number);

        /**
         * @brief Sets the customer's age
         * 
         * @param AN integer age 
         * @return true if age was successfully set
         */
        bool setAge(const int age);

        /**
         * @brief Links a new account to the customer
         * 
         * @param A shared pointers to a BankAccount object  to add to the customer account
         */
        void linkAccount(std::shared_ptr<BankAccount> account);

        /**
         * @brief Checks if the customer has at least one credit card
         * 
         * @note Does NOT validate card status (active/expired) - only checks existence
         * @return true if the customer has one or more, false otherwise
         */
        bool hasCreditCard()const;

        /**
         * @brief Retrieves a list of saving accounts associated with the customer
         * 
         * @return A vector containing a shared pointers to customer's SavingAccount objects
         */
        std::vector<std::shared_ptr<SavingAccount>> getSavingAccount() const;

        /**
         * @brief Retrives a list of checking accounts associated with the customer
         * 
         * @return A vector containing a shared pointers to customer's CheckingAccount objects
         */
        std::vector<std::shared_ptr<CheckingAccount>> getCheckingAccount() const;

        /**
         * @brief Retrieve customer id (using inline declaration)
         * 
         * @return A string Id randomly generated
         */
        std::string getCustomerId() const;

        /**
         * @brief Removes an account from the customer's account list by account number
         * 
         * @param account_number The account number of the BankAccount to remove
         * @return true if the account was found and removed, false otherwise
         */
        bool removeAccount(int account_number);

        /**
         * @brief Sets the customer's address
         * 
         * @param a const string reference address 
         * @return true if address was successfully set
         */
        bool setAddress(const std::string& address);

        /**
         * @brief Gets the customer's address
         * 
         * @return A string representing the customer's address
         */
        std::string getAddress() const;
        
        /**
         * @brief Adds a BankAccount to the customer's account list
         * 
         * @param account A shared pointer to the BankAccount to add
         */
        void addAccount(std::shared_ptr<BankAccount> account);
        
        /**
         * @brief Retrieves the first 8 characters of the customer's ID
         * 
         * @return A string representing the short customer ID
         */
        std::string getShortCustomerId() const;
            
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
        std::vector<std::shared_ptr<Card>> getAllCards() const;

};

#endif 