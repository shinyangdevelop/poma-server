#include <iostream>
#include <boost/asio.hpp>
#include <cstring>

#include "models/Packets.h"

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;

        // 1. Resolve and connect to the local server
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "8080");

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);
        std::cout << "Connected to the server!\n";

        // 2. Prepare the Login Request Body
        PktLoginReq reqBody;
        // strncpy cleanly copies the string and ensures the rest of the 16 bytes are null-padded
        std::strncpy(reqBody.szID, "admin", sizeof(reqBody.szID) - 1);
        std::strncpy(reqBody.szPW, "secret", sizeof(reqBody.szPW) - 1);

        // 3. Prepare the Header
        PktHeader reqHeader;
        reqHeader.TotalSize = sizeof(PktHeader) + sizeof(PktLoginReq);
        reqHeader.Id = static_cast<uint16_t>(PACKET_ID::USER_LOGIN_REQ);
        reqHeader.Reserve = 0;

        // 4. Send the Header and Body
        // Synchronous write will block until all bytes are pushed to the network
        boost::asio::write(socket, boost::asio::buffer(&reqHeader, sizeof(PktHeader)));
        boost::asio::write(socket, boost::asio::buffer(&reqBody, sizeof(PktLoginReq)));

        std::cout << "-> Sent USER_LOGIN_REQ (" << reqHeader.TotalSize << " bytes) for user '" << "admin" << "'" << "\n";

        // 5. Read the Response Header
        PktHeader resHeader;
        boost::asio::read(socket, boost::asio::buffer(&resHeader, sizeof(PktHeader)));

        // 6. Check the ID and Read the Response Body
        if (resHeader.Id == static_cast<uint16_t>(PACKET_ID::USER_LOGIN_RES)) {
            PktLoginRes resBody;
            boost::asio::read(socket, boost::asio::buffer(&resBody, sizeof(PktLoginRes)));

            std::cout << "<- Received USER_LOGIN_RES (" << resHeader.TotalSize << " bytes)\n";
            std::cout << "------------------------------\n";

            // Cast the enum to an int just so std::cout can print it as a number
            int statusCode = static_cast<int>(resBody.Status);
            std::cout << "Status: " << statusCode;

            if (resBody.Status == models::ErrorCode::SUCCESS) {
                std::cout << " (SUCCESS)\n";
                std::cout << "Coins : " << resBody.currentCoins << "\n";
                std::cout << "Level : " << resBody.currentLevel << "\n";
            } else {
                std::cout << " (FAILED)\n";
            }
            std::cout << "------------------------------\n";
        } else {
            std::cout << "Received unexpected packet ID: " << resHeader.Id << "\n";
        }

        // Socket closes automatically when it goes out of scope
    } catch (std::exception& e) {
        std::cerr << "Network Exception: " << e.what() << "\n";
    }

    return 0;
}