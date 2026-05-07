#pragma once
#include <cstdint>

#pragma pack(push, 1)

enum class PACKET_ID : uint16_t {
    NONE = 0,
    USER_LOGIN_REQ = 21,
    USER_LOGIN_RES = 22,

    MAX = 256,
};

struct PktHeader {
    uint16_t TotalSize;
    uint16_t Id;
    uint8_t Reserve;
};

constexpr int MAX_USER_ID_SIZE = 16;
constexpr int MAX_USER_PASSWORD_SIZE = 16;

struct PktLoginReq {
    char szID[MAX_USER_ID_SIZE] = {0, };
    char szPW[MAX_USER_PASSWORD_SIZE] = {0, };
};

struct PktLoginRes {
    bool IsSuccess = false;
    int currentCoins = 0;
    int currentLevel = 0;
}

#pragma pack(pop)