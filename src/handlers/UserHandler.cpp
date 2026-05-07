#include "handlers/UserHandler.h"
#include <iostream>
#include "database/DatabaseManager.h"
#include "core/UserManager.h"
#include <optional>

namespace handlers {
    UserHandler::UserHandler(db::DatabaseManager *pDatabaseManager, core::UserManager *pUserManager) {
        std::cout << "UserHandler initialized.\n";
        m_pRefDbManager = pDatabaseManager;
        m_pUserManager = pUserManager;
    }

    std::string UserHandler::ProcessRequest(const std::string &requestData) {
        std::cout << "UserHandler processing: " << requestData << "\n";
        return requestData;
    }

    bool UserHandler::ProcessLogin(const PktLoginReq &reqPkt, int sessionIndex) const {
        const std::string ID =  std::string(reqPkt.szID, strnlen(reqPkt.szID, sizeof(reqPkt.szID)));
        const std::string PW =  std::string(reqPkt.szPW, strnlen(reqPkt.szPW, sizeof(reqPkt.szPW)));
        if (!ValidateUser(ID, PW)) {
            return false;
        }
        std::optional<PlayerProfile> profileOpt = GetPlayerProfile(ID);
        if (!profileOpt.has_value()) {
            std::cerr << "No account found: " << ID << "\n";
            return false;
        }
        m_pUserManager->AddUser(sessionIndex, profileOpt.value());
        return true;
    }

    // Private method implementation
    bool UserHandler::ValidateUser(const std::string &ID, const std::string &PW) const {
        const std::string sql = "SELECT * FROM accounts WHERE loginid=$1 AND password=$2;";
        const pqxx::result res = m_pRefDbManager->Query(sql, ID, PW);
        if (res.empty()) {
            return false;
        }
        return true;
    }

    std::optional<PlayerProfile> UserHandler::GetPlayerProfile(const std::string &ID) const {
        try {
            const std::string sql =
                    "SELECT p.account_id, p.display_name, p.current_coins, p.current_level FROM player_profiles p INNER JOIN accounts a ON p.account_id = a.id WHERE a.loginid = $1";
            const pqxx::result res = m_pRefDbManager->Query(sql, ID);
            if (res.empty()) {
                std::cout << "No profile found for user: " << ID << "\n";
                return std::nullopt;
            }
            PlayerProfile profile;
            profile.accountId = res[0]["account_id"].as<int>();
            profile.currentCoins = res[0]["current_coins"].as<int>();
            profile.currentLevel = res[0]["current_level"].as<int>();

            if (!res[0]["display_name"].is_null()) {
                profile.displayName = res[0]["display_name"].as<std::string>();
            } else {
                profile.displayName = "Unknown";
            }
            return profile;
        } catch (const std::exception &e) {
            std::cerr << "Database Query Error: " << e.what() << "\n";
            return std::nullopt;
        }
    }
} // namespace handlers
