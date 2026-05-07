#include "handlers/UserHandler.h"
#include <iostream>
#include "database/DatabaseManager.h"

namespace handlers {

    UserHandler::UserHandler(db::DatabaseManager *pDatabaseManager) {
        std::cout << "UserHandler initialized.\n";
        m_pRefDbManager = pDatabaseManager;
    }

    std::string UserHandler::ProcessRequest(const std::string& requestData) {
        std::cout << "UserHandler processing: " << requestData << "\n";
        return requestData;
    }

    bool UserHandler::ProcessLogin(const PktLoginReq &reqPkt) const {
        if (!ValidateUser(reqPkt.szID, reqPkt.szPW)) {
            return false;
        }
        return true;
    }

    // Private method implementation
    bool UserHandler::ValidateUser(const string& ID, const string& PW) const {
        const std::string sql = "SELECT * FROM users WHERE loginid=$1 AND password=$2;";
        const pqxx::result res = m_pRefDbManager->Query(sql, ID, PW);
        if (res.empty()) {
            return false;
        }
        return true;
    }
} // namespace handlers