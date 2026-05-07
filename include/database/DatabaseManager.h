#pragma once
#include <pqxx/pqxx>
#include <string>
#include <memory>
#include <mutex>
#include <iostream>

namespace db {
    class DatabaseManager {
    public:
        bool Connect(const std::string& connectionString);
        void Disconnect();
        template<typename... Args>
        pqxx::result Query(const std::string& sql, Args&&... args) {
            try {
                if (!conn_ || !conn_->is_open()) {
                    throw std::runtime_error("Database connection is not open.");
                }
                std::lock_guard<std::mutex> lock(db_mutex_);

                pqxx::work txn(*conn_);
                pqxx::result res = txn.exec_params(sql, args...);
                txn.commit();

                return res;
            }
            catch (pqxx::broken_connection& e) {
                if (Connect(connectionString_)) {
                    return Query(sql, std::forward<Args>(args)...);
                }
                std::cerr << "[DatabaseConnection]" << e.what() << std::endl;
                return pqxx::result{};
            }
            catch (std::exception& e) {
                std::cerr << "[DatabaseConnection]" << e.what() << std::endl;
                return pqxx::result{};
            }
        }

        template<typename... Args>
        int Execute(const std::string& sql, Args&&... args) {
            if (!conn_ || !conn_->is_open()) {
                throw std::runtime_error("Database connection is not open.");
            }
            std::lock_guard<std::mutex> lock(db_mutex_);

            pqxx::work txn(*conn_);
            pqxx::result res = txn.exec_params(sql, args...);
            txn.commit();

            return res.affected_rows();
        }
    private:
        std::unique_ptr<pqxx::connection> conn_;
        std::mutex db_mutex_;
        std::string connectionString_;
    };
}