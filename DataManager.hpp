#ifndef DATAMANAGER_HPP
#define DATAMANAGER_HPP

#include "AccountDataInterface.hpp"
#include "TransactionData.hpp"
#include "Customers.hpp"
#include <memory>
#include <vector>

class DataManager {
public:
    DataManager(std::unique_ptr<AccountDataInterface> accountData,
               std::unique_ptr<TransactionData> transactionData);

    // Account operations
    std::shared_ptr<BankAccount> loadAccount(int accountNumber);
    bool saveAccount(const BankAccount& account);
    bool removeAccount(int accountNumber);
    std::vector<std::shared_ptr<BankAccount>> findAccountsByCustomer(
        const std::string& customerId);

    // Transaction operations
    bool logTransaction(const Transaction& transaction);
    std::vector<Transaction> getTransactionsForAccount(
        const std::string& accountNumber);

    // Customer operations
    void addCustomer(std::shared_ptr<Customers> customer);
    bool removeCustomer(const std::string& customerId);
    std::shared_ptr<Customers> getCustomer(const std::string& customerId);
    std::vector<std::shared_ptr<Customers>> getAllCustomers();

private:
    std::unique_ptr<AccountDataInterface> accountData_;
    std::unique_ptr<TransactionData> transactionData_;
    std::vector<std::shared_ptr<Customers>> customers_;
};

#endif // DATAMANAGER_HPP