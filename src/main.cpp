#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include "models/Packets.h"
#include "handlers/PacketProcess.h"

using boost::asio::ip::tcp;

// Handles a single client connection
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket) : socket_(std::move(socket)) {}

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
                else {
                    std::cout << "Clinet Disconnected.";
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
    Server(boost::asio::io_context& io_context, short port)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "New client connected!\n";
                    std::make_shared<Session>(std::move(socket))->start();
                }
                do_accept(); // Keep listening for more clients
            });
    }

    tcp::acceptor acceptor_;
};

int main() {
    try {
        boost::asio::io_context io_context;
        Server server(io_context, 8080);

        std::cout << "Boost Async TCP Server running on port 8080...\n";
        io_context.run(); // This blocks and runs the event loop

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}