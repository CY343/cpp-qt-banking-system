#ifndef ACCOUNTDATA_HPP
#define ACCOUNTDATA_HPP

#include <memory>
#include <vector>
#include "BankAccount.hpp"

/**
 * @brief CRTP base class for AccountData implementations
 * @tparam Derived The concrete implementation class
 */
template <typename Derived>
class AccountData {
public:
    virtual ~AccountData() = default;

    std::shared_ptr<BankAccount> load(int accountNumber) {
        return static_cast<Derived*>(this)->loadImpl(accountNumber);
    }

    bool save(const BankAccount& account) {
        return static_cast<Derived*>(this)->saveImpl(account);
    }

    bool remove(int accountNumber) {
        return static_cast<Derived*>(this)->removeImpl(accountNumber);
    }

    std::vector<std::shared_ptr<BankAccount>> findByCustomer(const std::string& customerId) {
        return static_cast<Derived*>(this)->findByCustomerImpl(customerId);
    }
};

#endif // ACCOUNTDATA_HPP
