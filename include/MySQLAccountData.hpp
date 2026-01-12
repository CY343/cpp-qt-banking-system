#ifndef MYSQLACCOUNTDATA_HPP
#define MYSQLACCOUNTDATA_HPP

#include "AccountData.hpp"
#include "ConnectionPool.hpp"
#include"CheckingAccount.hpp"
#include"SavingAccount.hpp"
#include "BankingExceptions.hpp"
#include <memory>

#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

class MySQLAccountData : public AccountData<MySQLAccountData> {
public:
    explicit MySQLAccountData(std::shared_ptr<ConnectionPool> pool);
    
    // CRTP implementation methods
    std::shared_ptr<BankAccount> loadImpl(int accountNumber);
    bool saveImpl(const BankAccount& account);
    bool removeImpl(int accountNumber);
    std::vector<std::shared_ptr<BankAccount>> findByCustomerImpl(
        const std::string& customerId);

private:
    std::shared_ptr<ConnectionPool> pool_;
    
    // Helper methods
   std::shared_ptr<BankAccount> parseResult(sql::ResultSet* res);
    void bindParameters(
        sql::PreparedStatement* stmt, 
        const BankAccount& account);
};

#endif // MYSQLACCOUNTDATA_HPP