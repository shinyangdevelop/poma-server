#pragma once
#include "models/Packets.h"
#include <vector>
#include <functional>
#include <memory>

#include "UserHandler.h"
#include "core/Matchmaker.h"
#include "core/UserManager.h"
#include "database/DatabaseManager.h"
#include "handlers/RoomHandler.h"

namespace handlers {
    class PacketProcess {
    public:
        void Init(db::DatabaseManager *pDbManager, core::UserManager *pUserManager, core::Matchmaker *pMatchmaker, handlers::RoomHandler *pRoomHandler);

        void Process(short packetId, const char *pBodyData, size_t bodySize, int sessionIndex,
                     const std::function<void(int, std::vector<char>)> &routeCallback) const;

        template<typename T>
        void Send(PACKET_ID packetId, const T& data, int targetSessionIndex,
                  const std::function<void(int, std::vector<char>)> &routeCallback) const {

            size_t totalPacketSize = sizeof(PktHeader) + sizeof(T);
            std::vector<char> sendBuffer(totalPacketSize);

            auto *outHeader = reinterpret_cast<PktHeader*>(sendBuffer.data());
            outHeader->TotalSize = static_cast<uint16_t>(totalPacketSize);
            outHeader->Id = static_cast<short>(packetId);
            outHeader->Reserve = 0;

            auto *outBody = reinterpret_cast<T*>(sendBuffer.data() + sizeof(PktHeader));
            *outBody = data;

            routeCallback(targetSessionIndex, std::move(sendBuffer));
        }

    private:
        db::DatabaseManager *m_pRefDbManager = nullptr;
        core::UserManager *m_pUserManager = nullptr;

        core::Matchmaker *m_pMatchmaker = nullptr;

        handlers::RoomHandler *m_pRoomHandler = nullptr;

        std::unique_ptr<UserHandler> m_pUserHandler;

    };
}