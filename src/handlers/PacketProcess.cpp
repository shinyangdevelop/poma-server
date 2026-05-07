#include "handlers/PacketProcess.h"
#include "models/Packets.h"
#include <iostream>
#include "handlers/UserHandler.h"

namespace handlers {
    void PacketProcess::Process(short packetId, const char *pBodyData, size_t bodySize, int sessionIndex) const {
        switch (packetId) {
            case (short) PACKET_ID::USER_LOGIN_REQ: {
                if (bodySize < sizeof(PktLoginReq)) {
                    std::cerr << "[Security] Client " << sessionIndex << " sent a malformed LOGIN_REQ. Dropping.\n";
                    return;
                }
                const auto reqPkt = reinterpret_cast<const PktLoginReq *>(pBodyData);
                std::cout << m_pUserHandler -> ProcessLogin(*reqPkt, sessionIndex) << '\n';
                break;
            }
            default: {
                std::cout << "Unknown Packet Id" << packetId << std::endl;
                break;
            }
        }
    }

    void PacketProcess::Init(db::DatabaseManager *pDbManager, core::UserManager *pUserManager) {
        m_pRefDbManager = pDbManager;
        m_pUserManager = pUserManager;
        m_pUserHandler = std::make_unique<UserHandler>(m_pRefDbManager, m_pUserManager);
    }
}
