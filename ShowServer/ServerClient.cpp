// Copyright © 2024 Charles Kerr. All rights reserved.

#include "ServerClient.hpp"


// =======================================================================
auto ServerClient::closeCallback(ConnectionPointer conn) -> void {
    if (remoteCloseCallback != nullptr) {
        remoteCloseCallback(this->shared_from_this()) ;
    }
}

// =======================================================================
auto ServerClient::processCallback(PacketPointer packet , ConnectionPointer conn) -> bool {
    auto id = packet->packetID() ;
    // We handle identification internally
    auto iter = std::find_if(packetRoutines.begin(),packetRoutines.end(),[id](const std::pair<PacketType::PacketID, PacketFunction> &entry){
        return id == entry.first ;
    });
    if (iter != packetRoutines.end()) {
        // We found one!
        return iter->second(this->shared_from_this(),packet);
    }
    return true ;
}
// =======================================================================
ServerClient::ServerClient(ConnectionPointer connection, const PacketRoutines &routines) : remoteCloseCallback(nullptr) {
    packetRoutines = routines ;
    this->connection = connection ;
    this->connection->setCloseCallback(std::bind(&ServerClient::closeCallback,this,std::placeholders::_1));
    this->connection->setPacketRoutine(std::bind(&ServerClient::processCallback,this,std::placeholders::_1,std::placeholders::_2));
    
}
// =======================================================================
ServerClient::~ServerClient() {
    if (connection->is_open()) {
        connection->close() ;
    }
}

// =======================================================================
auto ServerClient::send(const Packet &packet) -> bool {
    if (connection == nullptr || !connection->is_open()){
        return false ;
    }
    return connection->send(packet) ;
}

// =======================================================================
auto ServerClient::is_open() const -> bool {
    if (connection == nullptr) {
        return false ;
    }
    return connection->is_open() ;
}

// =======================================================================
auto ServerClient::close() -> void {
    if (connection == nullptr) {
        return  ;
    }
    connection->close() ;
}

// =======================================================================
auto ServerClient::setHandle(const std::string &name) -> void {
    connection->handle = name ;
}

// =======================================================================
auto ServerClient::setRemoteClose(RemoteCloseRoutine &function)-> void {
    this->remoteCloseCallback = function ;
}
