#include <iostream>
#include <boost/asio.hpp>
#include <cstring>
#include "models/Packets.h"

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);

        // 1. Connect to the local server
        std::cout << "Connecting to 127.0.0.1:8080...\n";
        socket.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 8080));
        std::cout << "Connected!\n";

        // 2. Prepare the Packet Body
        PktLoginReq loginBody;
        std::strncpy(loginBody.szID, "admin_cpp", MAX_USER_ID_SIZE);
        std::strncpy(loginBody.szPW, "supersecret", MAX_USER_PASSWORD_SIZE);

        // 3. Prepare the Packet Header
        PktHeader header;
        header.TotalSize = sizeof(PktHeader) + sizeof(PktLoginReq);
        header.Id = (short)PACKET_ID::USER_LOGIN_REQ;
        header.Reserve = 0;

        // 4. Send the data using Scatter-Gather I/O
        // This takes our two structs and sends them back-to-back as one byte stream
        std::array<boost::asio::const_buffer, 2> buffers = {
            boost::asio::buffer(&header, sizeof(header)),
            boost::asio::buffer(&loginBody, sizeof(loginBody))
        };

        boost::asio::write(socket, buffers);
        std::cout << "LOGIN_IN_REQ packet sent successfully! (" << header.TotalSize << " bytes)\n";

        // (Optional) If you want to wait for a response from the server,
        // you would read it here before closing the socket.

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}