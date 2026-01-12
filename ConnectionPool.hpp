#ifndef CONNECTIONPOOL_HPP
#define CONNECTIONPOOL_HPP

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <atomic>
#include <thread>

class ConnectionPool
{
public:
    // Singleton access
    static ConnectionPool& getInstance();
    
    // Configuration structure
    struct Config {
        std::string url;
        std::string user;
        std::string password;
        std::string schema;
        size_t poolSize = 10;
        size_t maxWaitMs = 5000;     // Max wait time in milliseconds
        bool enableSSL = false;
        std::string sslCA;
        std::string sslCert;
        std::string sslKey;
        std::string validationQuery = "SELECT 1";
        bool verifySSL = true;  // Add certificate validation flag
    };
    
    // Initialize with configuration
    void initialize(const Config& config);
    
    // Get a connection from the pool
    std::shared_ptr<sql::Connection> getConnection();
    
    // Return a connection to the pool
    void returnConnection(std::shared_ptr<sql::Connection> conn);
    
    // Destructor
    ~ConnectionPool();
    
    // Delete copy operations
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;
    
    // Move operations
    ConnectionPool(ConnectionPool&& other) noexcept;
    ConnectionPool& operator=(ConnectionPool&& other) noexcept;

private:
    // Private constructor
    ConnectionPool() = default;
    
    // Create a new connection with SSL options
    std::shared_ptr<sql::Connection> createNewConnection();
    
    // Validate connection health
    bool validateConnection(std::shared_ptr<sql::Connection> conn);
    
    // Background maintenance thread
    void maintenanceThread();
    
    // Members
    sql::mysql::MySQL_Driver* driver_ = nullptr;
    std::queue<std::shared_ptr<sql::Connection>> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;
    size_t currentSize_ = 0;
    Config config_;
    std::atomic<bool> running_{false};
    std::thread maintenance_thread_;
};

#endif // CONNECTIONPOOL_HPP