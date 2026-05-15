#include "handlers/RoomHandler.h"
#include <iostream>
#include <algorithm>
#include <random>

namespace handlers {

    // Helper function to build a full 136-tile Mahjong wall
    std::vector<game::CARD> GenerateShuffledWall(int seed) {
        std::vector<game::CARD> wall;
        // There are 34 tile types, and 4 copies of each tile
        for (int i = 0; i < static_cast<int>(game::CARD::MAX_CARDS); ++i) {
            for (int j = 0; j < 4; ++j) {
                wall.push_back(static_cast<game::CARD>(i));
            }
        }
        std::mt19937 rng(seed);
        std::shuffle(wall.begin(), wall.end(), rng);
        return wall;
    }

    int RoomHandler::CreateRoom(const std::vector<int>& players, int randomSeed) {
        if (players.size() != 4) {
            std::cerr << "[RoomHandler] A Mahjong room requires exactly 4 players!\n";
            return -1;
        }

        std::lock_guard<std::mutex> lock(roomMutex_);

        int newRoomId = nextRoomId_++;
        auto newRoom = std::make_shared<Room>();
        newRoom->roomId = newRoomId;

        // Assign the 4 session IDs
        for (int i = 0; i < 4; i++) {
            std::cout << newRoom->sessionIds[i] << ' ' << players[i] << '\n';
            newRoom->sessionIds[i] = players[i];
        }

        // Generate the wall and boot up the Mahjong game engine!
        std::vector<game::CARD> initialWall = GenerateShuffledWall(randomSeed);
        newRoom->gameManager = std::make_unique<game::MahjongGameManager>(initialWall, randomSeed % 12 + 1);

        // Save it to our active rooms map
        rooms_[newRoomId] = newRoom;

        std::cout << "[RoomHandler] Room " << newRoomId << " created and game started!\n";
        return newRoomId;
    }

    void RoomHandler::DeleteRoom(int roomId) {
        std::lock_guard<std::mutex> lock(roomMutex_);

        auto erased = rooms_.erase(roomId);
        if (erased) {
            std::cout << "[RoomHandler] Room " << roomId << " has been destroyed.\n";
        }
    }
} // namespace handlers