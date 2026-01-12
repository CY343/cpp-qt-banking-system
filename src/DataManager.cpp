#include "DataManager.hpp"
#include <algorithm>

DataManager::DataManager(std::unique_ptr<AccountDataInterface> accountData,
                         std::unique_ptr<TransactionData> transactionData)
    : accountData_(std::move(accountData)),
      transactionData_(std::move(transactionData)) {}

// Account operations
std::shared_ptr<BankAccount> DataManager::loadAccount(int accountNumber) {
    return accountData_->load(accountNumber);
}

bool DataManager::saveAccount(const BankAccount& account) {
    return accountData_->save(account);
}

bool DataManager::removeAccount(int accountNumber) {
    return accountData_->remove(accountNumber);
}

std::vector<std::shared_ptr<BankAccount>> DataManager::findAccountsByCustomer(
    const std::string& customerId) {
    return accountData_->findByCustomer(customerId);
}

// Transaction operations
bool DataManager::logTransaction(const Transaction& transaction) {
    return transactionData_->log(transaction);
}

std::vector<Transaction> DataManager::getTransactionsForAccount(
    const std::string& accountNumber) {
    return transactionData_->getByAccount(accountNumber);
}

// Customer operations
void DataManager::addCustomer(std::shared_ptr<Customers> customer) {
    customers_.push_back(std::move(customer));
}

bool DataManager::removeCustomer(const std::string& customerId) {
    auto it = std::remove_if(customers_.begin(), customers_.end(),
        [&customerId](const auto& customer) {
            return customer->getCustomerId() == customerId;
        });
    
    if (it != customers_.end()) {
        customers_.erase(it, customers_.end());
        return true;
    }
    return false;
}

std::shared_ptr<Customers> DataManager::getCustomer(const std::string& customerId) {
    auto it = std::find_if(customers_.begin(), customers_.end(),
        [&customerId](const auto& customer) {
            return customer->getCustomerId() == customerId;
        });
    
    return (it != customers_.end()) ? *it : nullptr;
}

std::vector<std::shared_ptr<Customers>> DataManager::getAllCustomers() {
    return customers_;
}