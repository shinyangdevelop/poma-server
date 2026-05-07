#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <cstdlib>
#include "models/Packets.h"
#include "handlers/PacketProcess.h"

using boost::asio::ip::tcp;

// Handles a single client connection
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, db::DatabaseManager* m_pRefDbManager) : socket_(std::move(socket)) {
        processor_.Init(m_pRefDbManager);
    }

    void start() {
        do_read_header();
    }

private:
    void do_read_header() {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
            boost::asio::buffer(&header_, sizeof(PktHeader)),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    do_read_body();
                }
            });
    }

    void do_read_body() {
        auto self(shared_from_this());

        short body_size = header_.TotalSize - sizeof(PktHeader);
        body_buffer_.resize(body_size);

        boost::asio::async_read(socket_,
            boost::asio::buffer(body_buffer_.data(), body_size),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    processor_.Process(header_.Id, body_buffer_.data());
                    do_read_header();
                }
        });
    }

    tcp::socket socket_;
    PktHeader header_;
    std::vector<char> body_buffer_;
    handlers::PacketProcess processor_;
};

// Listens for new connections and creates Sessions
class Server {
public:
    Server(boost::asio::io_context& io_context, short port, db::DatabaseManager* dbManager)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), dbManager_(dbManager) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<Session>(std::move(socket), dbManager_)->start();
                }
                do_accept(); // Keep listening for more clients
            });
    }

    tcp::acceptor acceptor_;
    db::DatabaseManager* dbManager_;
};

int main() {
    try {
        db::DatabaseManager dbManager;
        const std::string dbUrl = std::getenv("DB_URL");
        if (!dbManager.Connect(dbUrl)) {
            std::cerr << "Failed to connect to database!\n";
            return -1;
        }

        boost::asio::io_context io_context;
        const std::string serverPort = std::getenv("SERVER_PORT");
        Server server(io_context, (short)stoi(serverPort), &dbManager);

        std::cout << "Boost Async TCP Server running on port 8080...\n";
        io_context.run(); // This blocks and runs the event loop

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}