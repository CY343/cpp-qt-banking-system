#include"MySQLTransactionData.hpp"
#include<cppconn/prepared_statement.h>


MySQLTransactionData::MySQLTransactionData(std::shared_ptr<ConnectionPool> pool): pool_(std::move(pool)){}

bool MySQLTransactionData::log(const Transaction& t)
{
    auto conn = pool_->getConnection();
    try
    {
    auto stmt = conn -> prepareStatement("INSERT INTO transactions(account_number, type, amount, timestamp)"
                                         "VALUES (?, ?, ?, ?)");
    stmt->setString(1, t.getAccountNumber());
    std::string typeStr = TransactionTypeToString(t.getType());
    stmt ->setString(2, typeStr);
    stmt->setDouble(3, t.getAmount());
    std::string datetimeStr = TimeToSQLDataTime(t.getTimeStamp());
    stmt -> setString(4, datetimeStr);
    return stmt -> executeUpdate() > 0;
}
    catch(sql::SQLException& e)
    {
        std::cerr << "SQL Error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Transaction> MySQLTransactionData::getByAccount(const std::string &accountNumber)
{
    std::vector<Transaction> transactions;
    auto conn = pool_->getConnection();

    try
    {
        auto stmt = conn->prepareStatement("SELECT type, amount, timestamp From transactions "
                                            "Where account_number = ? ORDER BY timestamp DESC");
        stmt->setString(1, accountNumber);

        auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
        while(res->next())
        {
            transactions.emplace_back(accountNumber, Transaction::stringToType(res->getString("type")),
                                                                               res->getDouble("amount"),
                                                                               static_cast<time_t>(res->getInt64("timestamp")));
        
        }
    }
    catch(const sql::SQLException& e)
    {
        std::cerr << "MySQL error in getByAccount: " << e.what() << std::endl;
        return{};
    }
    catch(const std::exception& e)
    {
        std::cerr << "Standard expection in getByAccount(): " << e.what() << std::endl;
        return{};
    }
}

std::string MySQLTransactionData::TransactionTypeToString(Transaction::Type type)
{
    switch(type)
    {
        case Transaction::DEPOSIT:
            return "DEPOSIT";
        case Transaction::WITHDRAWAL:
            return "WITHDRAWAL";
        case Transaction::INTEREST:
            return "INTEREST";
        case Transaction::FEE:
            return "FEE";
        case Transaction::PURCHASE:
            return "PURCHASE";
        default:
            return "UNKNOWN";
    }
}

std::string MySQLTransactionData::TimeToSQLDataTime(time_t time)
{
    struct tm *timeinfo = localtime(&time);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S",timeinfo);
    return std::string(buffer);
}

