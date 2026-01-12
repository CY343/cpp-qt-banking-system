#include "Services.hpp"

#include "DebitCard.hpp"
#include "CreditCard.hpp"
#include "CheckingAccount.hpp"
#include "SavingAccount.hpp"

#include <algorithm>
#include <stdexcept>
#include <random>
#include <iostream>

// Qt hashing (used for manager password hashing)
#include <QCryptographicHash>
#include <QByteArray>

/**
 * @brief Hash a string using SHA-256 (Qt).
 *
 * @param s Input string.
 * @return SHA-256 digest bytes.
 */
static QByteArray sha256(const std::string& s)
{
    return QCryptographicHash::hash(QByteArray::fromStdString(s), QCryptographicHash::Sha256);
}

/**
 * @brief Validate manager credentials.
 *
 * Current behavior:
 * - username must be "Manager"
 * - password must match SHA-256 hash of "admin123"
 *
 * @param username Entered username.
 * @param password Entered password.
 * @return True if manager credentials match.
 */
bool Services::isManagerLogin(const std::string& username, const std::string& password) const
{
    const std::string adminUser = "Manager";
    static const QByteArray adminPassHash = sha256("admin123");

    return (username == adminUser) && (sha256(password) == adminPassHash);
}

/**
 * @brief Authenticate a user (Manager or Customer).
 *
 * Manager path:
 * - If isManagerLogin returns true, outCustomer is reset and role is Manager.
 *
 * Customer path:
 * - Finds a customer whose name matches username and checks password.
 *   (Current implementation uses a fixed password check: password == "1234".)
 *
 * @param username Entered username.
 * @param password Entered password.
 * @param outCustomer Output customer pointer if customer login succeeds.
 * @return LoginRole::Manager, LoginRole::Customer, or LoginRole::invalid.
 */
Services::LoginRole Services::authenticate(const std::string& username,
                                          const std::string& password,
                                          std::shared_ptr<Customers>& outCustomer) const
{
    if (isManagerLogin(username, password)) {
        outCustomer.reset();
        return LoginRole::Manager;
    }

    for (const auto& c : customers_) {
        if (c && c->getName() == username && password == "1234") {
            outCustomer = c;
            return LoginRole::Customer;
        }
    }

    outCustomer.reset();
    return LoginRole::invalid;
}

// (Optional) hashing/equality helpers for shared_ptr<Customers> based on customer ID.
// Not currently used in the given implementation.
struct CustomerPtrHash {
    size_t operator()(const std::shared_ptr<Customers>& c) const noexcept {
        if (!c) return 0;
        return std::hash<std::string>{}(c->getCustomerId());
    }
};

struct CustomerPtrEqual {
    bool operator()(const std::shared_ptr<Customers>& lhs,
                    const std::shared_ptr<Customers>& rhs) const noexcept {
        if (!lhs || !rhs) return lhs == rhs;
        return lhs->getCustomerId() == rhs->getCustomerId();
    }
};

/**
 * @brief Generate a random 4-digit PIN string (0000-9999).
 *
 * @return 4-character PIN string.
 */
static std::string generateRandomPIN()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dist(0, 9);

    std::string pin;
    for (int i = 0; i < 4; ++i) {
        pin += std::to_string(dist(gen));
    }
    return pin;
}

/**
 * @brief Default constructor.
 */
Services::Services() {}

/**
 * @brief Add multiple customers to the system.
 *
 * Appends the provided customers to customers_.
 *
 * @param NewCustomers Customers to add.
 * @return True if added successfully.
 */
bool Services::addCustomers(const std::vector<std::shared_ptr<Customers>>& NewCustomers)
{
    customers_.insert(customers_.end(), NewCustomers.begin(), NewCustomers.end());
    return true;
}

/**
 * @brief Get the list of customers.
 * @return Reference to customers_.
 */
const std::vector<std::shared_ptr<Customers>>& Services::getCustomers() const
{
    return customers_;
}

/**
 * @brief Delete a customer from the system.
 *
 * Searches by pointer equality and erases if found.
 *
 * @param CustomersToDelete Customer pointer to delete.
 * @return True if deleted, false otherwise.
 */
bool Services::deleteCustomers(const std::shared_ptr<Customers>& CustomersToDelete)
{
    if (!CustomersToDelete) return false;

    auto it = std::find_if(customers_.begin(), customers_.end(),
                           [&CustomersToDelete](const std::shared_ptr<Customers>& c) {
                               return c == CustomersToDelete;
                           });

    if (it != customers_.end()) {
        customers_.erase(it);
        return true;
    }

    return false;
}

/**
 * @brief Close an account by account number.
 *
 * - Unlinks it from any customer that holds it.
 * - Removes it from the global registry all_accounts_.
 *
 * @param account_number Account number to close.
 * @return True if account was found and removed, false otherwise.
 */
bool Services::closeAccount(int account_number)
{
    bool unlinked = false;

    // 1) Remove account from customers' profiles
    for (auto& customer : customers_) {
        if (!customer) continue;

        std::cout << "Checking customer " << customer->getName() << std::endl;

        if (customer->removeAccount(account_number)) {
            std::cout << "Unlinked account " << account_number << " from customer.\n";
            unlinked = true;
        }
    }

    if (!unlinked) {
        std::cerr << "Account #" << account_number << " not found in any customer profiles\n";
        return false;
    }

    // 2) Remove from system registry
    auto it = std::find_if(all_accounts_.begin(), all_accounts_.end(),
                           [account_number](const auto& acc) {
                               return acc && acc->getAccountNumber() == account_number;
                           });

    if (it == all_accounts_.end()) {
        std::cerr << "Account #" << account_number << " not found in system registry\n";
        return false;
    }

    all_accounts_.erase(it);
    return true;
}

/**
 * @brief Process a transaction on an account.
 *
 * Deposits call applyDeposit(amount).
 * Withdrawals call applyWithdraw(amount).
 *
 * @param account Account to operate on.
 * @param type Transaction type.
 * @param amount Amount to deposit/withdraw.
 * @return True if transaction succeeds, false if it throws.
 */
bool Services::processTransaction(std::shared_ptr<BankAccount> account,
                                 Transaction::Type type,
                                 double amount)
{
    try {
        if (type == Transaction::DEPOSIT) {
            account->applyDeposit(amount);
        } else {
            account->applyWithdraw(amount);
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Transaction failed: " << e.what() << "\n";
        return false;
    }
}

/**
 * @brief Replace an old card with a new one.
 *
 * Behavior:
 * - If old_card is invalid or nullptr, returns nullptr.
 * - Deactivates and expires the old card.
 * - Creates a new card of the same dynamic type:
 *   - DebitCard: same linked account + same withdrawal limit + new random PIN.
 *   - CreditCard: same linked account + same credit limit, activated.
 *
 * @param old_card Card to replace.
 * @return New card instance or nullptr on failure.
 */
std::shared_ptr<Card> Services::replaceCard(std::shared_ptr<Card> old_card)
{
    if (!old_card || !old_card->validate()) return nullptr;

    old_card->setActivated(false);
    old_card->markExpired();

    if (auto debit = std::dynamic_pointer_cast<DebitCard>(old_card)) {
        auto account = debit->getLinkedAccount().lock();
        if (!account) return nullptr;

        auto new_debit = std::make_shared<DebitCard>(
            account,
            debit->getDailyWithdrawalLimit(),
            generateRandomPIN()
        );
        return new_debit;
    }
    else if (auto credit = std::dynamic_pointer_cast<CreditCard>(old_card)) {
        auto account = credit->getLinkedAccount().lock();
        if (!account) return nullptr;

        auto new_credit = std::make_shared<CreditCard>(
            credit->getCreditLimit(),
            true,
            account
        );
        return new_credit;
    }

    return nullptr;
}

/**
 * @brief Apply interest to all savings accounts.
 *
 * Iterates through all_accounts_ and applies interest to SavingAccount objects.
 *
 * @note Your SavingAccount class must provide the method called here.
 * If your savings method is named applyMonthlyInterest() (common), update the call accordingly.
 */
void Services::applyMonthlyInterestToAll()
{
    for (auto& account : all_accounts_) {
        if (auto savings = std::dynamic_pointer_cast<SavingAccount>(account)) {
            // If your class uses applyMonthlyInterest(), change this call to match.
            savings->applyInterest();
        }
    }
}

/**
 * @brief Process monthly fees for all checking accounts.
 *
 * Iterates through all_accounts_ and applies maintenance fees to CheckingAccount objects.
 */
void Services::processMonthlyFees()
{
    for (auto& account : all_accounts_) {
        if (auto checking = std::dynamic_pointer_cast<CheckingAccount>(account)) {
            checking->applyMonthlyMaintenanceFee();
        }
    }
}

/**
 * @brief Get the registry of all accounts.
 * @return Reference to all_accounts_.
 */
const std::vector<std::shared_ptr<BankAccount>>& Services::getAllAccounts() const
{
    return all_accounts_;
}

/**
 * @brief Register an account in the system.
 * @param account Account to add.
 * @return True if added successfully.
 */
bool Services::addAccount(std::shared_ptr<BankAccount> account)
{
    all_accounts_.push_back(account);
    return true;
}
