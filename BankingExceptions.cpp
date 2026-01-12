#include "BankingExceptions.hpp"
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <iostream>

TransactionGuard::TransactionGuard(void* conn) 
    : conn_(conn), committed_(false) 
{
    if (conn_) {
        static_cast<sql::Connection*>(conn_)->setAutoCommit(false);
    }
}

void TransactionGuard::commit() {
    if (committed_ || !conn_) return;
    try {
        static_cast<sql::Connection*>(conn_)->commit();
        committed_ = true;
    } catch (const sql::SQLException& e) {
        std::cerr << "Commit failed: " << e.what() << std::endl;
    }
}

TransactionGuard::~TransactionGuard() {
    if (!committed_ && conn_) {
        try {
            static_cast<sql::Connection*>(conn_)->rollback();
            static_cast<sql::Connection*>(conn_)->setAutoCommit(true);
        } catch (const sql::SQLException& e) {
            std::cerr << "Rollback failed: " << e.what() << std::endl;
        }
    }
}
