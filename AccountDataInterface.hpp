#ifndef ACCOUNTDATAINTERFACE_HPP
#define ACCOUNTDATAINTERFACE_HPP

#include <memory>
#include <vector>
#include "BankAccount.hpp"

class AccountDataInterface {
public:
    virtual ~AccountDataInterface() = default;
    virtual std::shared_ptr<BankAccount> load(int accountNumber) = 0;
    virtual bool save(const BankAccount& account) = 0;
    virtual bool remove(int accountNumber) = 0;
    virtual std::vector<std::shared_ptr<BankAccount>> findByCustomer(
        const std::string& customerId) = 0;
};

#endif // ACCOUNTDATAINTERFACE_HPP