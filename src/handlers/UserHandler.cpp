#include "handlers/UserHandler.h"
#include <iostream>

namespace handlers {

    UserHandler::UserHandler() {
        std::cout << "UserHandler initialized.\n";
    }

    std::string UserHandler::processRequest(const std::string& requestData) {
        std::cout << "UserHandler processing: " << requestData << "\n";

        if (validateUser(requestData)) {
            return "SUCCESS: User validated\n";
        } else {
            return "ERROR: Invalid user data\n";
        }
    }

    // Private method implementation
    bool UserHandler::validateUser(const std::string& data) {
        // Dummy logic: assume valid if the string isn't empty
        return !data.empty() && data[0] != '1';
    }

} // namespace handlers