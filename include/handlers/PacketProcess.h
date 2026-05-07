#pragma once
#include "models/Packets.h"
#include <iostream>

#include "database/DatabaseManager.h"

namespace handlers {
    class PacketProcess {
    public:
        void Init(db::DatabaseManager* pDbManager);
        void Process(short packetId, const char* pBodyData);
    private:
        db::DatabaseManager* m_pRefDbManager = nullptr;
    };
}
