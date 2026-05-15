#pragma once
#include <deque>
#include <vector>
#include <mutex>
#include <memory>

#include "handlers/RoomHandler.h"

namespace core {
    
    // A clean struct to hand back to the network layer
    struct MatchResult {
        bool matchFound = false;
        int roomId = -1;
        std::vector<int> players;
    };

    class Matchmaker {
    public:
        Matchmaker(handlers::RoomHandler* roomHandler);
        
        MatchResult AddPlayerToQueue(int sessionIndex);
        
        void RemovePlayerFromQueue(int sessionIndex);

    private:
        std::deque<int> waitingQueue_;
        std::mutex queueMutex_;
        
        handlers::RoomHandler* pRoomHandler_;
    };
}