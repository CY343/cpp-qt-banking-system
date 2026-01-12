#ifndef SERVICES_HPP
#define SERVICES_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_set>
#include <type_traits>          // needed for std::is_base_of_v, std::is_same_v
#include <stdexcept>            // needed for std::invalid_argument

#include "Customers.hpp"
#include "BankAccount.hpp"
#include "CreditCard.hpp"
#include "SavingAccount.hpp"
#include "CheckingAccount.hpp"  // needed because openAccount references CheckingAccount

/**
 * @class Services
 * @brief Central service layer for managing customers, accounts, and card replacement.
 *
 * This class acts as a "system manager" that stores:
 * - a registry of customers
 * - a registry of all accounts in the system
 *
 * It provides:
 * - authentication (manager vs customer)
 * - adding/removing customers
 * - opening/closing accounts
 * - processing transactions (deposit/withdraw)
 * - replacing a card (DebitCard or CreditCard)
 * - monthly batch operations (interest and fees)
 */
class Services {
private:
    std::vector<std::shared_ptr<Customers>> customers_;      ///< All registered customers.
    std::vector<std::shared_ptr<BankAccount>> all_accounts_; ///< All accounts opened in the system.

    /**
     * @brief Checks if given credentials match the manager login.
     *
     * Manager login uses a fixed username ("Manager") and a hashed password ("admin123")
     * in the implementation.
     *
     * @param username Entered username.
     * @param password Entered password (plaintext).
     * @return True if credentials match manager account, false otherwise.
     */
    bool isManagerLogin(const std::string& username, const std::string& password) const;

public:
    /**
     * @enum LoginRole
     * @brief Authentication result categories.
     */
    enum class LoginRole { invalid, Customer, Manager };

    /**
     * @brief Authenticate a user as either Manager or Customer.
     *
     * - If manager credentials match, sets @p outCustomer to nullptr and returns Manager.
     * - Otherwise, searches customers_ by name and checks password (currently hardcoded check).
     *
     * @param username Entered username.
     * @param password Entered password.
     * @param outCustomer Output: set to matching customer if customer login succeeds.
     * @return LoginRole indicating result.
     */
    LoginRole authenticate(const std::string& username,
                           const std::string& password,
                           std::shared_ptr<Customers>& outCustomer) const;

    /**
     * @brief Default constructor.
     *
     * Initializes empty customer and account registries.
     */
    Services();

    /**
     * @brief Adds an account to the global registry.
     *
     * @param account Account to register.
     * @return True if added successfully.
     */
    bool addAccount(std::shared_ptr<BankAccount> account);

    /**
     * @brief Adds multiple customers to the system.
     *
     * @param NewCustomers Customers to add.
     * @return True if added successfully.
     */
    bool addCustomers(const std::vector<std::shared_ptr<Customers>>& NewCustomers);

    /**
     * @brief Removes a customer from the system.
     *
     * Searches by pointer equality in the current implementation.
     *
     * @param CustomersToDelete Customer object to remove.
     * @return True if removed, false if not found or nullptr.
     */
    bool deleteCustomers(const std::shared_ptr<Customers>& CustomersToDelete);

    /**
     * @brief Processes a deposit or withdrawal on an account.
     *
     * Calls applyDeposit or applyWithdraw on the provided account depending on @p type.
     * Exceptions are caught and reported; returns false on failure.
     *
     * @param account Account to operate on.
     * @param type Transaction type (deposit or withdraw).
     * @param amount Transaction amount.
     * @return True if transaction succeeds, false otherwise.
     */
    bool processTransaction(std::shared_ptr<BankAccount> account,
                            Transaction::Type type,
                            double amount);

    /**
     * @brief Returns the system-wide registry of all accounts.
     * @return Reference to all_accounts_.
     */
    const std::vector<std::shared_ptr<BankAccount>>& getAllAccounts() const;

    /**
     * @brief Returns the system-wide registry of customers.
     * @return Reference to customers_.
     */
    const std::vector<std::shared_ptr<Customers>>& getCustomers() const;

    /**
     * @brief Opens a new account of type T for a customer.
     *
     * Requirements:
     * - T must derive from BankAccount.
     * - Customer must be at least 18 years old (throws if underage).
     *
     * Behavior:
     * - Creates the new account with forwarded constructor args.
     * - Stores it in the global registry.
     * - Links it into the customer profile.
     * - If T is CheckingAccount, issues a DebitCard automatically.
     *
     * @tparam T BankAccount-derived type (e.g., CheckingAccount, SavingAccount).
     * @tparam Args Constructor argument pack for T.
     * @param customer Customer who owns the account.
     * @param args Arguments forwarded to T's constructor.
     * @return Shared pointer to the created account.
     * @throws std::invalid_argument if customer is under 18.
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> openAccount(std::shared_ptr<Customers> customer, Args&&... args)
    {
        static_assert(std::is_base_of_v<BankAccount, T>, "T must be derived from BankAccount");

        if (customer->getAge() < 18) {
            throw std::invalid_argument("Customers must be at least 18 years old.");
        }

        auto new_account = std::make_shared<T>(std::forward<Args>(args)...);
        all_accounts_.push_back(new_account);
        customer->linkAccount(new_account);

        if constexpr (std::is_same_v<T, CheckingAccount>) {
            new_account->issueDebitCard();
        }

        return new_account;
    }

    /**
     * @brief Closes an account by account number.
     *
     * Steps:
     * 1) Unlinks account from all customers who have it.
     * 2) Removes it from global registry all_accounts_.
     *
     * @param account_number Account number to close.
     * @return True if removed successfully, false if not found.
     */
    bool closeAccount(int account_number);

    /**
     * @brief Replaces an existing card with a newly issued card.
     *
     * - Deactivates and marks the old card expired.
     * - If old card is DebitCard: creates a new DebitCard linked to same CheckingAccount
     *   with same withdrawal limit and a randomly generated PIN.
     * - If old card is CreditCard: creates a new CreditCard with same limit and linked account.
     *
     * @param old_card Card to replace.
     * @return Newly created card, or nullptr on failure.
     */
    std::shared_ptr<Card> replaceCard(std::shared_ptr<Card> old_card);

    /**
     * @brief Applies monthly interest to all savings accounts in the system.
     *
     * Iterates through all_accounts_ and applies interest to accounts that are SavingAccount.
     */
    void applyMonthlyInterestToAll();

    /**
     * @brief Processes monthly maintenance fees for all checking accounts.
     *
     * Iterates through all_accounts_ and calls the maintenance fee function on CheckingAccount.
     */
    void processMonthlyFees();
};

#endif
