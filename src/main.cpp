#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <cstdlib>
#include <atomic>

#include "core/UserManager.h"
#include "models/Packets.h"
#include "handlers/PacketProcess.h"

using boost::asio::ip::tcp;

// Handles a single client connection

static std::atomic<int> global_session_counter{1};

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, db::DatabaseManager *m_pRefDbManager,
            core::UserManager *pUserManager) : socket_(std::move(socket)),
                                               sessionIndex_(global_session_counter++),
                                               userManager_(pUserManager),
                                               header_(PktHeader()) {
        processor_.Init(m_pRefDbManager, userManager_);
    }

    void start() {
        do_read_header();
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
                                        userManager_ -> RemoveUser(sessionIndex_);
                                    }
                                });
    }

    void do_read_body() {
        auto self(shared_from_this());
        if (header_.TotalSize < sizeof(PktHeader) || header_.TotalSize > 8192) {
            std::cerr << "[Security] Client " << sessionIndex_ << " sent malicious packet size. Disconnecting.\n";
            socket_.close();
            userManager_->RemoveUser(sessionIndex_);
            return;
        }
        size_t body_size = header_.TotalSize - sizeof(PktHeader);
        body_buffer_.resize(body_size);

        boost::asio::async_read(socket_,
                                boost::asio::buffer(body_buffer_.data(), body_size),
                                [this, self](boost::system::error_code ec, std::size_t length) {
                                    if (!ec) {
                                        processor_.Process(header_.Id, body_buffer_.data(), length, sessionIndex_);
                                        do_read_header();
                                    } else {
                                        std::cout << "[Session] Client " << sessionIndex_ << " disconnected during body read.\n";
                                    }
                                });
    }

    tcp::socket socket_;
    PktHeader header_;
    std::vector<char> body_buffer_;
    handlers::PacketProcess processor_;
    int sessionIndex_;
    core::UserManager *userManager_;
};

// Listens for new connections and creates Sessions
class Server {
public:
    Server(boost::asio::io_context &io_context, short port, db::DatabaseManager *dbManager,
           core::UserManager *pUserManager)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), dbManager_(dbManager), userManager_(pUserManager) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket), dbManager_, userManager_)->start();
                }
                do_accept(); // Keep listening for more clients
            });
    }

    tcp::acceptor acceptor_;
    db::DatabaseManager *dbManager_;
    core::UserManager *userManager_;
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
        Server server(io_context, static_cast<short>(stoi(serverPort)), &dbManager, &userManager);

        std::cout << "Boost Async TCP Server running on port 8080...\n";
        io_context.run(); // This blocks and runs the event loop
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
