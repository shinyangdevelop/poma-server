#include "handlers/PacketProcess.h"
#include "models/Packets.h"
#include <iostream>
#include "handlers/UserHandler.h"

namespace handlers {
    void PacketProcess::Process(short packetId, const char *pBodyData, size_t bodySize, int sessionIndex,
                                const std::function<void(int targetSessionIndex, std::vector<char>)> &routeCallback)
    const {
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
                    m_pUserManager->ReadUser(sessionIndex, [&resBody](const PlayerProfile &profile) {
                        resBody.Status = models::ErrorCode::SUCCESS;
                        resBody.currentCoins = profile.currentCoins;
                        resBody.currentLevel = profile.currentLevel;
                        resBody.accountId = profile.accountId;
                    });
                } else {
                    // If they failed, tell the client why
                    resBody.Status = models::ErrorCode::INVALID_CREDENTIALS;
                }

                Send(PACKET_ID::USER_LOGIN_RES, resBody, sessionIndex, routeCallback);
                break;
            }
            case (short) PACKET_ID::QUEUE_JOIN_REQ: {
                if (bodySize < sizeof(PktQueueJoinReq)) {
                    std::cerr << "[Security] Client " << sessionIndex << " sent a malformed LOGIN_REQ. Dropping.\n";
                    return;
                }
                const auto reqPkt = reinterpret_cast<const PktQueueJoinReq *>(pBodyData);
                std::cout << reqPkt->userId << std::endl;

                core::MatchResult match = m_pMatchmaker->AddPlayerToQueue(sessionIndex);
                if (match.matchFound) {
                    PktMatchFoundRes resBody{};
                    resBody.roomId = match.roomId;
                    for (int playerId: match.players) {
                        Send(PACKET_ID::MATCH_FOUND_RES, resBody, playerId, routeCallback);
                    }
                }
                break;
            }
            default: {
                std::cout << "Unknown Packet Id" << packetId << std::endl;
                break;
            }
        }
    }

    void PacketProcess::Init(db::DatabaseManager *pDbManager, core::UserManager *pUserManager, core::Matchmaker *pMatchmaker, handlers::RoomHandler *pRoomHandler) {
        m_pRefDbManager = pDbManager;
        m_pUserManager = pUserManager;
        m_pMatchmaker = pMatchmaker;
        m_pRoomHandler = pRoomHandler;
        m_pUserHandler = std::make_unique<UserHandler>(m_pRefDbManager, m_pUserManager);
    }
}
