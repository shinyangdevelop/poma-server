#pragma once
#include <cstdint>

namespace models {
    enum class ErrorCode : uint8_t {
        CONNECTION_ERROR = 10,
        UNKNOWN_ERROR = 240,
    };
}