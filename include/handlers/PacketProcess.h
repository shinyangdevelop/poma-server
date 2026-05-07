#pragma once
#include "models/Packets.h"
#include <iostream>

#include "UserHandler.h"
#include "core/UserManager.h"
#include "database/DatabaseManager.h"

namespace handlers {
    class PacketProcess {
    public:
        void Init(db::DatabaseManager* pDbManager, core::UserManager *pUserManager);
        void Process(short packetId, const char* pBodyData, size_t bodySize, int sessionIndex) const;
    private:
        db::DatabaseManager* m_pRefDbManager;
        core::UserManager* m_pUserManager;
        std::unique_ptr<UserHandler> m_pUserHandler;
    };
}
