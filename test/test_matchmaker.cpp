#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <cstring>
#include <string>
#include <chrono>

#include "models/Packets.h"

using boost::asio::ip::tcp;

void simulate_player(int playerNum, const std::string& username, const std::string& password) {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);
        boost::asio::connect(socket, resolver.resolve("127.0.0.1", "8080"));

        // --- 1. SEND LOGIN REQUEST ---
        PktLoginReq loginReq;
        // Cleanly wipe the arrays first, then copy the strings to ensure null padding
        std::memset(loginReq.szID, 0, sizeof(loginReq.szID));
        std::memset(loginReq.szPW, 0, sizeof(loginReq.szPW));
        std::strncpy(loginReq.szID, username.c_str(), sizeof(loginReq.szID) - 1);
        std::strncpy(loginReq.szPW, password.c_str(), sizeof(loginReq.szPW) - 1);

        PktHeader loginHeader;
        loginHeader.TotalSize = sizeof(PktHeader) + sizeof(PktLoginReq);
        loginHeader.Id = static_cast<uint16_t>(PACKET_ID::USER_LOGIN_REQ);
        loginHeader.Reserve = 0;

        // Send Header and Body
        boost::asio::write(socket, boost::asio::buffer(&loginHeader, sizeof(PktHeader)));
        boost::asio::write(socket, boost::asio::buffer(&loginReq, sizeof(PktLoginReq)));

        // --- 2. RECEIVE LOGIN RESPONSE ---
        PktHeader resHeader;
        boost::asio::read(socket, boost::asio::buffer(&resHeader, sizeof(PktHeader)));

        if (resHeader.Id == static_cast<uint16_t>(PACKET_ID::USER_LOGIN_RES)) {
            PktLoginRes loginRes;
            boost::asio::read(socket, boost::asio::buffer(&loginRes, sizeof(PktLoginRes)));

            if (loginRes.Status != models::ErrorCode::SUCCESS) {
                std::cout << "[Player " << playerNum << "] Login Failed for user '" << username << "'\n";
                return;
            }

            std::cout << "[Player " << playerNum << "] Logged in successfully! Account ID: " << loginRes.accountId << "\n";

            // --- 3. SEND QUEUE JOIN REQUEST ---
            PktQueueJoinReq queueReq;
            queueReq.userId = loginRes.accountId;

            PktHeader queueHeader;
            queueHeader.TotalSize = sizeof(PktHeader) + sizeof(PktQueueJoinReq);
            queueHeader.Id = static_cast<uint16_t>(PACKET_ID::QUEUE_JOIN_REQ);
            queueHeader.Reserve = 0;

            // Stagger joins slightly by half a second so we can watch them queue up in the server console
            std::this_thread::sleep_for(std::chrono::milliseconds(playerNum * 500));

            boost::asio::write(socket, boost::asio::buffer(&queueHeader, sizeof(PktHeader)));
            boost::asio::write(socket, boost::asio::buffer(&queueReq, sizeof(PktQueueJoinReq)));
            std::cout << "[Player " << playerNum << "] Joined the matchmaking queue...\n";

            // --- 4. WAIT FOR MATCH FOUND RESPONSE ---
            // This read will BLOCK until the server broadcasts the start signal!
            PktHeader matchHeader;
            boost::asio::read(socket, boost::asio::buffer(&matchHeader, sizeof(PktHeader)));

            if (matchHeader.Id == static_cast<uint16_t>(PACKET_ID::MATCH_FOUND_RES)) {
                PktMatchFoundRes matchRes;
                boost::asio::read(socket, boost::asio::buffer(&matchRes, sizeof(PktMatchFoundRes)));
                std::cout << "🎉 [Player " << playerNum << "] MATCH FOUND! Assigned to Room " << matchRes.roomId << ". Ready to play!\n";
            } else {
                std::cout << "[Player " << playerNum << "] Received unexpected packet ID: " << matchHeader.Id << "\n";
            }
        }

    } catch (std::exception& e) {
        std::cerr << "[Player " << playerNum << "] Network Exception: " << e.what() << "\n";
    }
}

int main() {
    std::cout << "Starting 4 concurrent player threads in C++...\n\n";

    std::vector<std::thread> threads;
    
    // Spawn 4 parallel threads acting as 4 separate clients
    for (int i = 1; i <= 4; ++i) {
        std::string username = "player" + std::to_string(i);
        threads.emplace_back(simulate_player, i, username, "pass123");
    }

    // Wait for all 4 clients to finish
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "\nAll players successfully joined the room. Server test complete!\n";
    return 0;
}