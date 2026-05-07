#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>

struct PlayerProfile {
    std::string displayName;
    int accountId;
    int currentCoins = 0;
    int currentLevel = 0;
    bool needsSaving = false;
};

namespace core {
    class UserManager {
    public:
        bool AddUser(int sessionIndex, const PlayerProfile& profile);
        void RemoveUser(int sessionIndex);
        bool UpdateUser(int sessionIndex, const std::function<void(PlayerProfile&)> &updateLogic);
        bool ReadUser(int sessionIndex, const std::function<void(PlayerProfile&)> &readLogic);
    private:
        std::unordered_map<int, PlayerProfile> m_activeUsers;
        std::mutex m_userMutex;
    };
}