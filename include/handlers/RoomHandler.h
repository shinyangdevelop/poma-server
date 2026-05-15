#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <array>
#include <atomic>

#include "game/Mahjong.h"

namespace handlers {

    struct Room {
        int roomId;
        std::array<int, 4> sessionIds = {-1, -1, -1, -1};

        std::unique_ptr<game::MahjongGameManager> gameManager;
    };

    class RoomHandler {
    public:
        int CreateRoom(const std::vector<int>& players, int randomSeed);

        void DeleteRoom(int roomId);
    
    private:
        std::unordered_map<int, std::shared_ptr<Room>> rooms_;
        std::mutex roomMutex_;
        std::atomic<int> nextRoomId_{1}; // Automatically generates unique Room IDs
    };
}