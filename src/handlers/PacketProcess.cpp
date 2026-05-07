#include "handlers/PacketProcess.h"
#include "models/Packets.h"
#include <iostream>
#include "handlers/UserHandler.h"

namespace handlers {
    void PacketProcess::Process(short packetId, const char* pBodyData) {
        switch (packetId) {
            case (short)PACKET_ID::USER_LOGIN_REQ: {
                const auto reqPkt = reinterpret_cast<const PktLoginReq *>(pBodyData);
                 const UserHandler userHandler(m_pRefDbManager);
                cout << userHandler.ProcessLogin(*reqPkt) << '\n';
                break;
            }
            default: {
                std::cout << "Unknown Packet Id" << packetId << std::endl;
                break;
            }
        }
    }

    void PacketProcess::Init(db::DatabaseManager *pDbManager) {
        m_pRefDbManager = pDbManager;
    }
}
