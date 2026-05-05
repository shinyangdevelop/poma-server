#pragma once
#include "models/Packets.h"
#include <iostream>

namespace handlers {
    class PacketProcess {
    public:
        void Process(short packetId, const char* pBodyData) {
            switch (packetId) {
                case (short)PACKET_ID::USER_LOGIN_REQ:
                    Login(pBodyData);
                    break;

                default:
                    std::cout << "Unknown Packet Id" << packetId << std::endl;
                    break;
            }
        }
    private:
        void Login(const char* pData) {
            auto reqPkt = reinterpret_cast<const PktLoginReq*>(pData);
            std::cout << "USER_LOGIN_REQ : " << reqPkt->szID << ' ' << reqPkt -> szPW << std::endl;
        }
    };
}
