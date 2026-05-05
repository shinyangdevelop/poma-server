#pragma once
#include <string>

namespace handlers {
    class UserHandler {
    public:
        UserHandler();
        virtual ~UserHandler() = default;

        std::string processRequest(const std::string& requestData);

    private:
        bool validateUser(const std::string& data);
    };
}