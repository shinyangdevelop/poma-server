#include "core/Matchmaker.h"
#include <iostream>
#include <algorithm>

namespace core {
    Matchmaker::Matchmaker(handlers::RoomHandler* roomHandler) 
        : pRoomHandler_(roomHandler) {}

    MatchResult Matchmaker::AddPlayerToQueue(int sessionIndex) {
        std::lock_guard<std::mutex> lock(queueMutex_);
        
        if (std::find(waitingQueue_.begin(), waitingQueue_.end(), sessionIndex) != waitingQueue_.end()) {
            return MatchResult{false}; // Already in queue
        }

        waitingQueue_.push_back(sessionIndex);
        std::cout << "[Matchmaker] Session " << sessionIndex << " joined queue. Waiting: " 
                  << waitingQueue_.size() << "/4\n";

        if (waitingQueue_.size() >= 4) {
            MatchResult result;
            result.matchFound = true;

            
            for (int i = 0; i < 4; i++) {
                std::cout << waitingQueue_.front();
                result.players.push_back(waitingQueue_.front());
                waitingQueue_.pop_front();
            }

            int randomSeed = 12345; // WARNING: Fix this
            result.roomId = pRoomHandler_->CreateRoom(result.players, randomSeed);

            std::cout << "[Matchmaker] Match found! Room ID: " << result.roomId << " created.\n";
            return result;
        }

        return MatchResult{false};
    }

    void Matchmaker::RemovePlayerFromQueue(int sessionIndex) {
        std::lock_guard<std::mutex> lock(queueMutex_);
        
        // Find the player and remove them if they exist
        auto it = std::find(waitingQueue_.begin(), waitingQueue_.end(), sessionIndex);
        if (it != waitingQueue_.end()) {
            waitingQueue_.erase(it);
            std::cout << "[Matchmaker] Session " << sessionIndex << " left queue. Waiting: " 
                      << waitingQueue_.size() << "/4\n";
        }
    }
}