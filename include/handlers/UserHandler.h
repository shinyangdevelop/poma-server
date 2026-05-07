#pragma once
#include <string>
#include <optional>
#include "database/DatabaseManager.h"
#include "core/UserManager.h"
#include "models/Packets.h"

namespace handlers {
    class UserHandler {
    public:
        explicit UserHandler(db::DatabaseManager *pDatabaseManager, core::UserManager *pUserManager);

        static std::string ProcessRequest(const std::string& requestData);
        bool ProcessLogin(const PktLoginReq& requestData, int sessionIndex) const;

    private:
        bool ValidateUser(const std::string &ID, const std::string &PW) const;
        std::optional<PlayerProfile> GetPlayerProfile(const std::string &ID) const;
        db::DatabaseManager *m_pRefDbManager;
        core::UserManager *m_pUserManager;
    };
}