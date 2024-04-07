// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef ServerClient_hpp
#define ServerClient_hpp

#include <memory>
#include <unordered_map>
#include <functional>
#include <thread>
#include <string>

#include "asio.hpp"
#include "utility/timeutil.hpp"
#include "network/Connection.hpp"

class ServerClient ;

using ClientPointer = std::shared_ptr<ServerClient> ;
using PacketFunction = std::function<bool(ClientPointer,PacketPointer)> ;
using PacketRoutines = std::unordered_map<PacketType::PacketID, PacketFunction> ;


class ServerClient : public std::enable_shared_from_this<ServerClient>{
    using RemoteCloseRoutine = std::function<void(ClientPointer)> ;

    friend class Connection ;
    std::shared_ptr<Connection> connection ;
    auto closeCallback(ConnectionPointer conn) -> void ;
    auto processCallback(PacketPointer packet , ConnectionPointer conn) -> bool ;
    RemoteCloseRoutine remoteCloseCallback ;
    PacketRoutines packetRoutines ;
public:
    ServerClient(ConnectionPointer connection, const PacketRoutines &routines) ;
    ~ServerClient() ;
    auto send(const Packet &packet) -> bool ;
    auto is_open() const -> bool ;
    auto close() -> void ;
    auto setHandle(const std::string &name) -> void ;
    auto setRemoteClose(RemoteCloseRoutine function)-> void ;
    auto read() -> void ;
    auto ip() const -> std::string ;
    auto timeStamp() -> util::ourclock::time_point ;
    auto handle() const -> const std::string& ;
};
#endif /* ServerClient_hpp */
