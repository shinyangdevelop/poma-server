#pragma once
#include <string>

#include "database/DatabaseManager.h"
#include "models/Packets.h"

namespace handlers {
    class UserHandler {
    public:
        explicit UserHandler(db::DatabaseManager *pDatabaseManager);

        static std::string ProcessRequest(const std::string& requestData);
        bool ProcessLogin(const PktLoginReq& requestData) const;

    private:
        bool ValidateUser(const std::string &ID, const std::string &PW) const;
        db::DatabaseManager *m_pRefDbManager;
    };
}