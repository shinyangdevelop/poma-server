#include "handlers/PacketProcess.h"
#include "models/Packets.h"
#include <iostream>
#include "handlers/UserHandler.h"

namespace handlers {
    void PacketProcess::Process(short packetId, const char *pBodyData, size_t bodySize, int sessionIndex, std::function<void(std::vector<char>)> sendCallback) const {
        switch (packetId) {
            case (short) PACKET_ID::USER_LOGIN_REQ: {
                if (bodySize < sizeof(PktLoginReq)) {
                    std::cerr << "[Security] Client " << sessionIndex << " sent a malformed LOGIN_REQ. Dropping.\n";
                    return;
                }
                const auto reqPkt = reinterpret_cast<const PktLoginReq *>(pBodyData);

                // 1. Actually check if the login succeeded!
                bool isSuccess = m_pUserHandler->ProcessLogin(*reqPkt, sessionIndex);

                // 2. Prepare the response packet body
                PktLoginRes resBody;

                if (isSuccess) {
                    // 3. We use [&resBody] to capture our packet by reference so the lambda can modify it
                    // Note: Make sure it says 'const PlayerProfile&' to match your header!
                    m_pUserManager->ReadUser(sessionIndex, [&resBody](const PlayerProfile &profile) {
                        resBody.Status = models::ErrorCode::SUCCESS;
                        resBody.currentCoins = profile.currentCoins;
                        resBody.currentLevel = profile.currentLevel;
                    });
                } else {
                    // If they failed, tell the client why
                    resBody.Status = models::ErrorCode::INVALID_CREDENTIALS;
                }

                // 4. Create a raw byte buffer large enough for the Header + the Body
                size_t totalPacketSize = sizeof(PktHeader) + sizeof(PktLoginRes);
                std::vector<char> sendBuffer(totalPacketSize);

                // 5. Build the Header at the very front of the buffer
                auto *outHeader = reinterpret_cast<PktHeader *>(sendBuffer.data());
                outHeader->TotalSize = static_cast<uint16_t>(totalPacketSize);
                outHeader->Id = static_cast<uint16_t>(PACKET_ID::USER_LOGIN_RES);
                outHeader->Reserve = 0;

                // 6. Build the Body directly after the Header
                auto *outBody = reinterpret_cast<PktLoginRes *>(sendBuffer.data() + sizeof(PktHeader));
                *outBody = resBody;

                // 7. Fire the bytes back to the Session!
                sendCallback(sendBuffer);

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
