#include "ConnectionPool.hpp"
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <openssl/ssl.h>

using namespace std::chrono_literals;

ConnectionPool& ConnectionPool::getInstance() {
    static ConnectionPool instance;
    return instance;
}

ConnectionPool::~ConnectionPool() {
    running_ = false;
    
    if (maintenance_thread_.joinable()) {
        maintenance_thread_.join();
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    while (!pool_.empty()) {
        try {
            auto conn = pool_.front();
            conn->close();
            pool_.pop();
            currentSize_--;
        } catch (...) {
            // Ignore exceptions during destruction
        }
    }
}

ConnectionPool::ConnectionPool(ConnectionPool&& other) noexcept 
    : driver_(std::exchange(other.driver_, nullptr)),
      pool_(std::move(other.pool_)),
      mutex_(),
      cond_(),
      currentSize_(std::exchange(other.currentSize_, 0)),
      config_(std::move(other.config_)),
      running_(other.running_.load()),
      maintenance_thread_(std::move(other.maintenance_thread_)) 
{
    // Condition variables and mutexes are not movable, so we'll create new ones
}

ConnectionPool& ConnectionPool::operator=(ConnectionPool&& other) noexcept {
    if (this != &other) {
        // Stop current maintenance
        running_ = false;
        if (maintenance_thread_.joinable()) {
            maintenance_thread_.join();
        }
        
        // Clean current pool
        {
            std::lock_guard<std::mutex> lock(mutex_);
            while (!pool_.empty()) {
                try {
                    auto conn = pool_.front();
                    conn->close();
                    pool_.pop();
                } catch (...) {}
            }
            currentSize_ = 0;
        }
        
        // Move from other
        driver_ = std::exchange(other.driver_, nullptr);
        pool_ = std::move(other.pool_);
        currentSize_ = std::exchange(other.currentSize_, 0);
        config_ = std::move(other.config_);
        running_ = other.running_.load();
        maintenance_thread_ = std::move(other.maintenance_thread_);
    }
    return *this;
}

void ConnectionPool::initialize(const Config& config) {
    if (driver_ || running_) {
        throw std::logic_error("ConnectionPool already initialized");
    }
    
    // Validate configuration
    if (config.poolSize == 0) {
        throw std::invalid_argument("Pool size must be at least 1");
    }
    
    if (config.enableSSL && (config.sslCA.empty() || config.sslCert.empty() || config.sslKey.empty())) {
        throw std::invalid_argument("SSL requires CA, cert, and key paths");
    }
    
    config_ = config;
    driver_ = sql::mysql::get_driver_instance();
    
    // Initialize SSL if needed
    if (config_.enableSSL) {
        // Initialize OpenSSL once
        static std::once_flag ssl_init_flag;
        std::call_once(ssl_init_flag, []() {
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();
        });
    }
    
    // Create initial connections
    for (size_t i = 0; i < config_.poolSize; ++i) {
        try {
            auto conn = createNewConnection();
            if (validateConnection(conn)) {
                pool_.push(conn);
                currentSize_++;
            }
        } catch (const sql::SQLException& e) {
            std::cerr << "Failed to initialize connection #" << i 
                      << ": " << e.what() << std::endl;
        }
    }
    
    // Start maintenance thread
    running_ = true;
    maintenance_thread_ = std::thread(&ConnectionPool::maintenanceThread, this);
    
    if (pool_.empty()) {
        throw std::runtime_error("Failed to initialize any database connections");
    }
}

std::shared_ptr<sql::Connection> ConnectionPool::createNewConnection() {
    try {
        sql::ConnectOptionsMap properties;
        properties["hostName"] = config_.url;
        properties["userName"] = config_.user;
        properties["password"] = config_.password;
        properties["schema"] = config_.schema;
        properties["OPT_CONNECT_TIMEOUT"] = 5;
        properties["OPT_RECONNECT"] = true;
        
        if (config_.enableSSL) {
            properties["sslEnable"] = true;
            properties["sslCA"] = config_.sslCA;
            properties["sslCert"] = config_.sslCert;
            properties["sslKey"] = config_.sslKey;
            properties["sslVerify"] = true;
        }

        if (config_.verifySSL) {
                properties["sslVerify"] = true;
                properties["sslVerifyServerCert"] = true;
            }
        
        auto conn = std::shared_ptr<sql::Connection>(
            driver_->connect(properties),
            [this](sql::Connection* c) { this->returnConnection(std::shared_ptr<sql::Connection>(c)); }
        );
        
        return conn;
    } catch (const sql::SQLException& e) {
        throw std::runtime_error("SQL connection failed: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Connection failed: " + std::string(e.what()));
    }
}

bool ConnectionPool::validateConnection(std::shared_ptr<sql::Connection> conn) {
    if (!conn || conn->isClosed()) {
        return false;
    }
    
    try {
        std::unique_ptr<sql::Statement> stmt(conn->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery(config_.validationQuery));
        return res->next() && res->getInt(1) == 1;
    } catch (const sql::SQLException& e) {
        std::cerr << "Connection validation failed: " << e.what() << std::endl;
        return false;
    } catch (...) {
        return false;
    }
}

std::shared_ptr<sql::Connection> ConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(mutex_);
    const auto timeout_time = std::chrono::steady_clock::now() + 
                             std::chrono::milliseconds(config_.maxWaitMs);
    
    while (true) {
        // Case 1: Available connection in pool
        if (!pool_.empty()) {
            auto conn = pool_.front();
            pool_.pop();
            
            if (validateConnection(conn)) {
                return conn;
            } else {
                // Bad connection - close and reduce count
                try { conn->close(); } catch (...) {}
                currentSize_--;
            }
        }
        
        // Case 2: Can create new connection
        if (currentSize_ < config_.poolSize) {
            try {
                auto new_conn = createNewConnection();
                currentSize_++;
                return new_conn;
            } catch (const std::exception& e) {
                std::cerr << "Failed to create new connection: " << e.what() << std::endl;
            }
        }
        
        // Case 3: Wait with timeout
        if (cond_.wait_until(lock, timeout_time) == std::cv_status::timeout) {
            throw std::runtime_error("Connection request timed out after " + 
                                    std::to_string(config_.maxWaitMs) + "ms");
        }

        if (cond_.wait_until(lock, timeout_time) == std::cv_status::timeout) {
            throw std::runtime_error("Connection request timed out after " + 
                                    std::to_string(config_.maxWaitMs) + "ms");
        }
    }
}

void ConnectionPool::returnConnection(std::shared_ptr<sql::Connection> conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    try {
        if (validateConnection(conn)) {
            pool_.push(conn);
        } else {
            // Replace bad connection
            try { conn->close(); } catch (...) {}
            if (currentSize_ <= config_.poolSize) {
                try {
                    pool_.push(createNewConnection());
                } catch (...) {
                    currentSize_--;
                }
            } else {
                currentSize_--;
            }
        }
    } catch (...) {
        // Safety net
        try { conn->close(); } catch (...) {}
        currentSize_--;
    }
    
    cond_.notify_one();
}

void ConnectionPool::maintenanceThread() {
    while (running_) {
        // Sleep for 1 minute between maintenance cycles
        std::this_thread::sleep_for(60s);
        
        std::lock_guard<std::mutex> lock(mutex_);
        const size_t current_count = pool_.size();
        
        // Validate all connections in the pool
        for (size_t i = 0; i < current_count; ++i) {
            auto conn = pool_.front();
            pool_.pop();
            
            if (validateConnection(conn)) {
                pool_.push(conn);
            } else {
                // Replace bad connection
                try { conn->close(); } catch (...) {}
                try {
                    pool_.push(createNewConnection());
                } catch (...) {
                    currentSize_--;
                }
            }
        }
        
        // Refill pool if needed
        while (currentSize_ < config_.poolSize && pool_.size() < config_.poolSize) {
            try {
                pool_.push(createNewConnection());
                currentSize_++;
            } catch (...) {
                break;
            }
        }
    }
}