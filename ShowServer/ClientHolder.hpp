// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef ClientHolder_hpp
#define ClientHolder_hpp

#include <iostream>

#include <vector>
#include <mutex>

#include "ServerClient.hpp"
#include "packets/Packet.hpp"
#include "network/FrameValue.hpp"

class ClientHolder ;

using ClientHolderPointer = std::shared_ptr<ClientHolder> ;

class ClientHolder {
    std::vector<ClientPointer> clients ;
    mutable std::mutex accessControl ;
    
public:
    auto add(ClientPointer client) -> void ;
    auto send(const Packet &packet) -> void ;
    
    auto sendSync(const FrameValue &frame) -> void ;
    auto sendPlay(bool state, const FrameValue &frame) -> void ;
    auto sendNop() -> void ;
    auto sendLoad(const std::string &music, const std::string &light) -> void ;
    auto sendShow(bool state) -> void ;
    
    auto clear() -> void ;
};

#endif /* ClientHolder_hpp */
