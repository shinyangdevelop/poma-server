#pragma once

#include <vector>
#include <array>
#include <cstdint>

namespace game {
    // Locked to uint8_t for tiny memory footprint
    enum class CARD : short {
        MAN_1, MAN_2, MAN_3, MAN_4, MAN_5, MAN_6, MAN_7, MAN_8, MAN_9,
        PIN_1, PIN_2, PIN_3, PIN_4, PIN_5, PIN_6, PIN_7, PIN_8, PIN_9,
        SOU_1, SOU_2, SOU_3, SOU_4, SOU_5, SOU_6, SOU_7, SOU_8, SOU_9,
        WIND_EAST, WIND_WEST, WIND_NORTH, WIND_SOUTH,
        DRAGON_WHITE, DRAGON_BAL, DRAGON_MIDDLE,
        MAX_CARDS
    };

    // Bundle player data together!
    struct PlayerState {
        std::vector<CARD> closedHand;
        std::vector<CARD> discards;
        // In a full game, you'd also store 'Melds' (Chi/Pon/Kan) here

        bool isFuriten = false;
        int reachTurn = -1; // -1 means no reach, otherwise stores the turn they reached
        int points = 25000; // Standard starting points
    };

    class MahjongGameManager {
    public:
        MahjongGameManager(const std::vector<CARD> &preShuffledWall, int dice);

        // Core Game Loop Functions
        void DrawTile(int playerIndex);

        bool DiscardTile(int playerIndex, CARD tileToDiscard);

    protected:
        int jang = 0; // 0: East Round (동장), 1: South Round (남장)
        int currentTurn = 0; // 0: East, 1: South, 2: West, 3: North

        std::vector<CARD> wall_; // The remaining tiles to draw
        std::vector<CARD> deadWall_; // The 14 tiles at the end (Dora, etc.)

        std::array<PlayerState, 4> players_;

    private:
        // Action Checkers
        bool CanChi(int playerIndex, CARD discardedTile);

        bool CanPong(int playerIndex, CARD discardedTile);

        bool CanKkang(int playerIndex, CARD discardedTile);

        // Hand Evaluation
        bool IsTenpai(int playerIndex) const;

        // Utility
        void SortHand(int playerIndex);

        std::array<int, static_cast<int>(CARD::MAX_CARDS)> GetHandHistogram(int playerIndex) const;
    };
}
