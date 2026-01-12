#include "MySQLAccountData.hpp"
#include <memory>


MySQLAccountData::MySQLAccountData(std::shared_ptr<ConnectionPool> pool)
    : pool_(std::move(pool)) {}

std::shared_ptr<BankAccount> MySQLAccountData::loadImpl(int accountNumber) {
    auto conn = pool_->getConnection();
    try {
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement("SELECT * FROM accounts WHERE account_number = ?")
        );
        stmt->setInt(1, accountNumber);

        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
        if (res->next()) {
            return parseResult(res.get());  // Pass raw pointer
        }
        
        throw AccountException(
            "Account " + std::to_string(accountNumber) + " not found",
            AccountException::Type::NOT_FOUND
        );
    }
    catch (const AccountException&) {
        throw;
    }
    catch (const sql::SQLException& e) {
        throw DatabaseException(
            "MySQL error loading account [" + std::to_string(accountNumber) + "]: " + e.what(),
            DatabaseException::Operation::QUERY,
            e.getErrorCode()
        );
    }
    catch (const std::exception& e) {
        throw DatabaseException(
            "System error loading account [" + std::to_string(accountNumber) + "]: " + e.what(),
            DatabaseException::Operation::CONNECTION
        );
    }
}

bool MySQLAccountData::saveImpl(const BankAccount& account) {
    auto conn = pool_->getConnection();
    TransactionGuard guard(conn.get());
    bool success = false;
    
    try {
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "INSERT INTO accounts(account_number, balance, account_type) "
                "VALUES(?, ?, ?) "
                "ON DUPLICATE KEY UPDATE "
                "balance = VALUES(balance), "
                "account_type = VALUES(account_type)"
            )
        );
        bindParameters(stmt.get(), account);
        
        int affected = stmt->executeUpdate();
        success = (affected > 0);
        
        if (success) {
            guard.commit();
        }
        return success;
    }
    catch (const sql::SQLException& e) {
        throw DatabaseException(
            "MySQL error saving account [" + std::to_string(account.getAccountNumber()) + "]: " + e.what(),
            DatabaseException::Operation::QUERY,
            e.getErrorCode()
        );
    }
    catch (const std::exception& e) {
        throw DatabaseException(
            "System error saving account [" + std::to_string(account.getAccountNumber()) + "]: " + e.what(),
            DatabaseException::Operation::CONNECTION
        );
    }
}

bool MySQLAccountData::removeImpl(int accountNumber) {
    auto conn = pool_->getConnection();
    TransactionGuard guard(conn.get());
    bool success = false;
    
    try {
        // First remove customer associations
        std::unique_ptr<sql::PreparedStatement> linkStmt(
            conn->prepareStatement(
                "DELETE FROM customer_accounts WHERE account_number = ?"
            )
        );
        linkStmt->setInt(1, accountNumber);
        linkStmt->executeUpdate();
        
        // Then remove account
        std::unique_ptr<sql::PreparedStatement> accountStmt(
            conn->prepareStatement(
                "DELETE FROM accounts WHERE account_number = ?"
            )
        );
        accountStmt->setInt(1, accountNumber);
        
        int affected = accountStmt->executeUpdate();
        success = (affected > 0);
        
        if (success) {
            guard.commit();
        }
        return success;
    }
    catch (const sql::SQLException& e) {
        throw DatabaseException(
            "MySQL error deleting account [" + std::to_string(accountNumber) + "]: " + e.what(),
            DatabaseException::Operation::QUERY,
            e.getErrorCode()
        );
    }
    catch (const std::exception& e) {
        throw DatabaseException(
            "System error deleting account [" + std::to_string(accountNumber) + "]: " + e.what(),
            DatabaseException::Operation::CONNECTION
        );
    }
}

std::vector<std::shared_ptr<BankAccount>> MySQLAccountData::findByCustomerImpl(
    const std::string& customerId) 
{
    auto conn = pool_->getConnection();
    std::vector<std::shared_ptr<BankAccount>> accounts;
    
    try {
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn->prepareStatement(
                "SELECT a.account_number, a.balance, a.account_type "
                "FROM accounts a "
                "JOIN customer_accounts ca ON a.account_number = ca.account_number "
                "WHERE ca.customer_id = ?"
            )
        );
        stmt->setString(1, customerId);
        
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
        while (res->next()) {
            accounts.push_back(parseResult(res.get()));  // Pass raw pointer
        }
        return accounts;
    } 
    catch (const sql::SQLException& e) {
        throw DatabaseException(
            "MySQL error finding accounts for customer " + customerId + ": " + e.what(),
            DatabaseException::Operation::QUERY,
            e.getErrorCode()
        );
    }
    catch (const std::exception& e) {
        throw DatabaseException(
            "System error finding accounts for customer " + customerId + ": " + e.what(),
            DatabaseException::Operation::CONNECTION
        );
    }
}

std::shared_ptr<BankAccount> MySQLAccountData::parseResult(sql::ResultSet* res) {
    try {
        int accNum = res->getInt("account_number");
        double balance = res->getDouble("balance");
        std::string accType = res->getString("account_type");
        
        // Create appropriate account type
        if (accType == "SAVINGS") {
            return std::make_shared<SavingAccount>(accNum, balance, accType);
        } else if (accType == "CHECKING") {
            return std::make_shared<CheckingAccount>(accNum, balance, accType);
        } else {
            throw DatabaseException(
            "Unknown account type: " + accType,
            DatabaseException::Operation::QUERY,
            0
            );
        }
    } catch (const sql::SQLException& e) {
        throw DatabaseException(
            "Error parsing account data: " + std::string(e.what()),
            DatabaseException::Operation::QUERY,
            e.getErrorCode()
        );
    } catch (const std::exception& e) {
        throw DatabaseException(
            "System error parsing account data: " + std::string(e.what()),
            DatabaseException::Operation::CONNECTION
        );
    }
}

void MySQLAccountData::bindParameters(
    sql::PreparedStatement* stmt,
    const BankAccount& account) 
{
    stmt->setInt(1, account.getAccountNumber());
    stmt->setDouble(2, account.getAccountBalance());
    stmt->setString(3, account.getAccountType());
}