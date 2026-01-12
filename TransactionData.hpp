#ifndef TRANSACTIONDATA_HPP
#define TRANSACTIONDATA_HPP

#include"Transaction.hpp"
#include<vector>

class TransactionData
{
    public:
        virtual ~TransactionData() = default;
        virtual bool log(const Transaction& t) = 0;
        virtual std::vector<Transaction> getByAccount(const std::string& accountNumber) = 0;
};
#endif