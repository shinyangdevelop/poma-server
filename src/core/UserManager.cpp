#include "core/UserManager.h"
#include <iostream>

namespace core {
    bool UserManager::AddUser(int sessionIndex, const PlayerProfile& profile) {
        std::lock_guard<std ::mutex> lock(m_userMutex);

        if (m_activeUsers.contains(sessionIndex)) {
            std::cerr << "[UserManager] Session" << sessionIndex << " already exitsts." <<std::endl;
            return false;
        }

        for (const auto& pair : m_activeUsers) {
            if (pair.second.accountId == profile.accountId) {
                std::cerr << "[UserManager] User" << profile.accountId << " already exists." << std::endl;
                return false;
            }
        }

        m_activeUsers[sessionIndex] = profile;
        return true;
    }

    void UserManager::RemoveUser(int sessionIndex) {
        std::lock_guard<std::mutex> lock(m_userMutex);

        auto erasedAccount = m_activeUsers.erase(sessionIndex);
        if (erasedAccount) {
            std::cout << "[UserManager] Removed session " << sessionIndex << " from memory." << std::endl;
        }
    }

    bool UserManager::UpdateUser(int sessionIndex, const std::function<void(PlayerProfile&)> &updateLogic) {
        std::lock_guard<std::mutex> lock(m_userMutex);

        auto it = m_activeUsers.find(sessionIndex);
        if (it != m_activeUsers.end()) {
            updateLogic(it->second);
            return true;
        }
        return false;
    }

    bool UserManager::ReadUser(int sessionIndex, const std::function<void(PlayerProfile&)> &readLogic) {
        std::lock_guard<std::mutex> lock(m_userMutex);
        auto it = m_activeUsers.find(sessionIndex);
        if (it != m_activeUsers.end()) {
            readLogic(it->second);
            return true;
        }
        return false;
    }

    PlayerProfile *UserManager::GetUser(int sessionIndex) {
        std::lock_guard<std::mutex> lock(m_userMutex);

        auto it = m_activeUsers.find(sessionIndex);
        if (it != m_activeUsers.end()) {
            return (&it->second);
        }
        return nullptr;
    }
}
