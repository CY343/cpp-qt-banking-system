#ifndef MYSQLTRANSACTIONDATA_HPP
#define MYSQLTRANSACTIONDATA_HPP

#include"TransactionData.hpp"
#include"ConnectionPool.hpp"
#include<memory>

class MySQLTransactionData: public TransactionData
{
    public:
    explicit MySQLTransactionData(std::shared_ptr<ConnectionPool> pool);
    bool log(const Transaction& t) override;
    std::vector<Transaction> getByAccount(const std::string &accountNumber) override;

    private:
    std::shared_ptr<ConnectionPool> pool_;
    std::string TransactionTypeToString(Transaction::Type type);
    std::string TimeToSQLDataTime(time_t time);

};
#endif