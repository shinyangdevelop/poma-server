#include "game/mahjong.h"
#include <algorithm>
#include <iostream>

namespace game {
    MahjongGameManager::MahjongGameManager(const std::vector<CARD> &preShuffledWall, int dice) {
        wall_ = preShuffledWall;
        jang = 0;
        currentTurn = 0;

        for (int i = 0; i < 14; i++) {
            deadWall_.push_back(wall_[i]);
            wall_.pop_back();
        }
        for (int i = 0; i < 4; i++) {
            for (int t = 0; t < 13; t++) {
                players_[i].closedHand.push_back(wall_.back());
                wall_.pop_back();
            }
            SortHand(i);
        }
        DrawTile(currentTurn);
    }

    void MahjongGameManager::DrawTile(int playerIndex) {
        if (wall_.empty()) {
            std::cout << "Wall is empty";
            return;
        }
        CARD drawTile = wall_.back();
        wall_.pop_back();

        players_[playerIndex].closedHand.push_back(drawTile);

        std::cout << "Player " << playerIndex << " draw tile";

        // 쯔모 체크
    }

    bool MahjongGameManager::DiscardTile(int playerIndex, CARD tileToDiscard) {
        if (playerIndex != currentTurn) {
            std::cerr << "[Mahjong] It is not Player " << playerIndex << "'s turn!\n";
            return false;
        }

        auto &hand = players_[playerIndex].closedHand;

        const auto it = std::find(hand.begin(), hand.end(), tileToDiscard);
        if (it == hand.end()) {
            std::cerr << "[Mahjong] Player does not have this tile to discard!\n";
            return false;
        }

        hand.erase(it);
        SortHand(playerIndex);
        players_[playerIndex].discards.push_back(tileToDiscard);

        std::cout << "[Mahjong] Player " << playerIndex << " discarded tile " << static_cast<int>(tileToDiscard) <<
                "\n";

        // 치퐁깡 체크

        currentTurn = (currentTurn + 1) % 4;
        DrawTile(currentTurn);

        return true;
    }

    void MahjongGameManager::SortHand(int playerIndex) {
        std::sort(players_[playerIndex].closedHand.begin(), players_[playerIndex].closedHand.end());
    }

    std::array<int, static_cast<int>(CARD::MAX_CARDS)> MahjongGameManager::GetHandHistogram(int playerIndex) const {
        std::array<int, static_cast<int>(CARD::MAX_CARDS)> counts = {0};
        for (CARD tile: players_[playerIndex].closedHand) {
            counts[static_cast<int>(tile)]++;
        }
        return counts;
    }

    bool MahjongGameManager::CanPong(int playerIndex, CARD discardedTile) {
        auto counts = GetHandHistogram(playerIndex);
        if (counts[static_cast<int>(discardedTile)] >= 2) {
            return true;
        }
        return false;
    }

    bool MahjongGameManager::CanKkang(int playerIndex, CARD discardedTile) {
        auto counts = GetHandHistogram(playerIndex);
        if (counts[static_cast<int>(discardedTile)] >= 3) {
            return true;
        }
        return false;
    }

    bool MahjongGameManager::CanChi(int playerIndex, CARD discardedTile) {
        auto counts = GetHandHistogram(playerIndex);
        if (discardedTile == CARD::MAN_1 || discardedTile == CARD::PIN_1 || discardedTile == CARD::SOU_1) {
            return counts[static_cast<int>(discardedTile) + 1] && counts[static_cast<int>(discardedTile) + 2];
        }
        if (discardedTile == CARD::MAN_2 || discardedTile == CARD::PIN_2 || discardedTile == CARD::SOU_2) {
            return (counts[static_cast<int>(discardedTile) + 1] && counts[static_cast<int>(discardedTile) + 2]) ||
                   (counts[static_cast<int>(discardedTile) - 1] && counts[static_cast<int>(discardedTile) + 1]);
        }
        if (discardedTile == CARD::MAN_8 || discardedTile == CARD::PIN_8 || discardedTile == CARD::SOU_8) {
            return (counts[static_cast<int>(discardedTile) - 1] && counts[static_cast<int>(discardedTile) - 2]) ||
                   (counts[static_cast<int>(discardedTile) - 1] && counts[static_cast<int>(discardedTile) + 1]);
        }
        if (discardedTile == CARD::MAN_9 || discardedTile == CARD::PIN_9 || discardedTile == CARD::SOU_9) {
            return counts[static_cast<int>(discardedTile) - 1] && counts[static_cast<int>(discardedTile) - 2];
        }
        return (counts[static_cast<int>(discardedTile) - 1] && counts[static_cast<int>(discardedTile) - 2]) ||
               (counts[static_cast<int>(discardedTile) - 1] && counts[static_cast<int>(discardedTile) + 1]) ||
               (counts[static_cast<int>(discardedTile) + 1] && counts[static_cast<int>(discardedTile) + 2]);
    }

    bool MahjongGameManager::IsTenpai(int playerIndex) const {
        return false;
    }
}
