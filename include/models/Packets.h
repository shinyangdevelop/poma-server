#pragma once
#include <cstdint>

#include "Statuses.h"

#pragma pack(push, 1)

enum class PACKET_ID : uint16_t {
    NONE = 0,
    USER_LOGIN_REQ = 21,
    USER_LOGIN_RES = 22,

    QUEUE_JOIN_REQ = 31,
    MATCH_FOUND_RES = 32,

    MAX = 256,
};

struct PktHeader {
    short Id;
    uint16_t TotalSize;
    uint8_t Reserve;
};

constexpr int MAX_USER_ID_SIZE = 16;
constexpr int MAX_USER_PASSWORD_SIZE = 16;

struct PktLoginReq {
    char szID[MAX_USER_ID_SIZE] = {0, };
    char szPW[MAX_USER_PASSWORD_SIZE] = {0, };
};

struct PktLoginRes {
    models::ErrorCode Status = models::ErrorCode::INVALID_CREDENTIALS;
    int currentCoins = 0;
    int currentLevel = 0;
    int accountId = -1;
};

struct PktQueueJoinReq {
    int userId = 0;
};

struct PktMatchFoundRes {
    int roomId;
};

#pragma pack(pop)