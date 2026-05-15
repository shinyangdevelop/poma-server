#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <cstdlib>
#include <atomic>
#include <unordered_map>
#include <mutex>

#include "core/UserManager.h"
#include "models/Packets.h"
#include "handlers/PacketProcess.h"
#include "handlers/RoomHandler.h"
#include "core/Matchmaker.h"

using boost::asio::ip::tcp;

static std::atomic<int> global_session_counter{1};
using RouterCallback = std::function<void(int, std::vector<char>)>;
using DisconnectCallback = std::function<void(int)>;

// Handles a single client connection
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, db::DatabaseManager *m_pRefDbManager,
            core::UserManager *pUserManager, core::Matchmaker *pMatchmaker, handlers::RoomHandler *pRoomHandler,
            RouterCallback router, DisconnectCallback onDisconnect)
        : socket_(std::move(socket)),
          sessionIndex_(global_session_counter++),
          userManager_(pUserManager),
          routeCallback_(std::move(router)),
          disconnectCallback_(std::move(onDisconnect)),
          header_(PktHeader()) {

        // Initialize the processor with all 3 managers
        processor_.Init(m_pRefDbManager, userManager_, pMatchmaker, pRoomHandler);
    }

    int GetSessionId() const { return sessionIndex_; }

    void start() {
        do_read_header();
    }

    void SendPacket(std::vector<char> sendBuffer) {
        auto self(shared_from_this());

        auto bufferPtr = std::make_shared<std::vector<char>>(std::move(sendBuffer));
        boost::asio::async_write(socket_,
            boost::asio::buffer(bufferPtr->data(), bufferPtr->size()),
            [this, self, bufferPtr](const boost::system::error_code &ec, std::size_t length) {
                if (ec) {
                    std::cerr << "[Session] Write failed for client " << sessionIndex_ << "\n";
                }
            }
        );
    }

private:
    void do_read_header() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
            boost::asio::buffer(&header_, sizeof(PktHeader)),
            [this, self](const boost::system::error_code &ec, std::size_t length) {
                if (!ec) {
                    do_read_body();
                } else {
                    std::cout << "[Session] Client " << sessionIndex_ << " disconnected during header read.\n";
                    disconnectCallback_(sessionIndex_); // Centralized cleanup!
                }
            });
    }

    void do_read_body() {
        auto self(shared_from_this());
        if (header_.TotalSize < sizeof(PktHeader) || header_.TotalSize > 8192) {
            std::cerr << "[Security] Client " << sessionIndex_ << " sent malicious packet size. Disconnecting.\n";
            socket_.close();
            disconnectCallback_(sessionIndex_);
            return;
        }
        size_t body_size = header_.TotalSize - sizeof(PktHeader);
        body_buffer_.resize(body_size);

        boost::asio::async_read(socket_,
            boost::asio::buffer(body_buffer_.data(), body_size),
            [this, self](const boost::system::error_code &ec, std::size_t length) {
                if (!ec) {
                    // FIX: Pass the routeCallback_ into the processor so it can broadcast!
                    processor_.Process(header_.Id, body_buffer_.data(), length, sessionIndex_, routeCallback_);
                    do_read_header();
                } else {
                    std::cout << "[Session] Client " << sessionIndex_ << " disconnected during body read.\n";
                    disconnectCallback_(sessionIndex_); // Centralized cleanup!
                }
            });
    }

    tcp::socket socket_;
    PktHeader header_;
    std::vector<char> body_buffer_;
    handlers::PacketProcess processor_;
    int sessionIndex_;

    core::UserManager *userManager_;
    RouterCallback routeCallback_;
    DisconnectCallback disconnectCallback_;
};


// Listens for new connections and creates Sessions
class Server {
public:
    Server(boost::asio::io_context &io_context, short port, db::DatabaseManager *dbManager,
           core::UserManager *pUserManager)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
          dbManager_(dbManager),
          userManager_(pUserManager),
          matchmaker_(&roomHandler_) // Initialize Matchmaker with the RoomHandler
    {
        do_accept();
    }

    // THE ROUTER: Allows any Session to send a packet to any other Session
    void RoutePacket(int targetSessionId, std::vector<char> data) {
        std::lock_guard<std::mutex> lock(sessionsMutex_);
        auto it = activeSessions_.find(targetSessionId);
        if (it != activeSessions_.end()) {
            it->second->SendPacket(std::move(data));
        } else {
            std::cerr << "[Router] Attempted to route to disconnected session " << targetSessionId << "\n";
        }
    }

    // THE CLEANUP: Completely removes a dropped player from all game systems
    void HandleDisconnect(int sessionId) {
        userManager_->RemoveUser(sessionId);
        matchmaker_.RemovePlayerFromQueue(sessionId);

        std::lock_guard<std::mutex> lock(sessionsMutex_);
        activeSessions_.erase(sessionId); // Frees the RAM!
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](const boost::system::error_code &ec, tcp::socket socket) {
                if (!ec) {
                    // 1. Create our Lambda callbacks for the Session
                    auto router = [this](int targetId, std::vector<char> data) {
                        this->RoutePacket(targetId, std::move(data));
                    };

                    auto onDisconnect = [this](int targetId) {
                        this->HandleDisconnect(targetId);
                    };

                    // 2. Build the Session
                    auto newSession = std::make_shared<Session>(
                        std::move(socket), dbManager_, userManager_, &matchmaker_, &roomHandler_, router, onDisconnect
                    );

                    // 3. Save it to our map so the Router can find it later
                    {
                        std::lock_guard<std::mutex> lock(sessionsMutex_);
                        activeSessions_[newSession->GetSessionId()] = newSession;
                    }

                    // 4. Boot it up!
                    newSession->start();
                }
                do_accept(); // Keep listening for more clients
            });
    }

    tcp::acceptor acceptor_;
    db::DatabaseManager *dbManager_;
    core::UserManager *userManager_;

    // Game Systems
    handlers::RoomHandler roomHandler_;
    core::Matchmaker matchmaker_;

    // Live Sockets Map
    std::unordered_map<int, std::shared_ptr<Session>> activeSessions_;
    std::mutex sessionsMutex_;
};


int main() {
    try {
        db::DatabaseManager dbManager;
        core::UserManager userManager;

        const char* dbUrl = std::getenv("DB_URL");
        const char* serverPort = std::getenv("SERVER_PORT");
        if (dbUrl == nullptr || serverPort == nullptr ) {
            std::cerr << "FATAL: Environment variables DB_URL or SERVER_PORT are missing!\n";
            return -1;
        }
        if (!dbManager.Connect(dbUrl)) {
            std::cerr << "Failed to connect to database!\n";
            return -1;
        }

        boost::asio::io_context io_context;
        Server server(io_context, static_cast<short>(std::stoi(std::string(serverPort))), &dbManager, &userManager);

        std::cout << "Boost Async TCP Server running on port " << serverPort << "...\n";
        io_context.run(); // This blocks and runs the event loop

    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}