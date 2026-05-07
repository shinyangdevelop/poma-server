#pragma once
#include <cstdint>

namespace models {
    // 1-byte unsigned integer, perfect for network packets
    enum class ErrorCode : uint8_t {
        // --- 0: SUCCESS ---
        SUCCESS = 0,

        // --- 10-99: SERVER & CONNECTION ERRORS ---
        CONNECTION_ERROR = 10,
        DATABASE_ERROR = 11,
        MALFORMED_PACKET = 12,

        // --- 100-199: AUTHENTICATION ERRORS ---
        INVALID_CREDENTIALS = 100, // Wrong ID or Password
        ACCOUNT_NOT_FOUND = 101,   // DB connection worked, but no profile exists
        ALREADY_LOGGED_IN = 102,   // UserManager rejected them

        // --- 200+: GAMEPLAY ERRORS ---
        INSUFFICIENT_COINS = 200,
        LEVEL_TOO_LOW = 201,

        UNKNOWN_ERROR = 255, // Max value for uint8_t
    };
}