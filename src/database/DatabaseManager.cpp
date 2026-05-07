#include "database/DatabaseManager.h"

namespace db {
    bool DatabaseManager::Connect(const std::string& connectionString) {
        try {
            conn_ = std::make_unique<pqxx::connection>(connectionString);
            if (conn_ -> is_open()) {
                std::cout << "Successfully connected to PostgreSQL.\n";
                return true;
            }
        }
        catch (std::exception& e) {
            std::cerr << "Database Connection Failed: " << e.what() << '\n';
        }
        return false;
    }

    void DatabaseManager::Disconnect() const {
        if (conn_ && conn_ -> is_open()) {
            conn_ -> close();
            std::cout << "Database Disconnected.\n";
        }
    }
}