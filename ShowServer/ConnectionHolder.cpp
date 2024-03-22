// Copyright © 2024 Charles Kerr. All rights reserved.

#include "ConnectionHolder.hpp"

#include <functional>
#include <algorithm>
#include <fstream>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;
// =========================================================================
// ConnectionState
// =========================================================================

// =========================================================================
ConnectionState::ConnectionState(const std::string &connection_identity, const std::string &time_marker,const std::string &client_name,bool connection_state):identity(connection_identity),timestamp(time_marker),name(client_name),state(connection_state){}

// =========================================================================
auto ConnectionState::describe() const -> std::string {
    return name +" = "s + (state?"Connected"s:"Disconnected"s) + " , "s + timestamp +" , "s + identity ;
}

// =========================================================================
// StateHolder
// =========================================================================

// =========================================================================
auto StateHolder::add(ConnectionState &state) -> void {
    auto lock = std::lock_guard(access) ;
    states.push(state);
}

// =========================================================================
auto StateHolder::retrieve() -> std::queue<ConnectionState> {
    auto empty = std::queue<ConnectionState>() ;
    {
        auto lock = std::lock_guard(access) ;
        std::swap(empty,states) ;
    }
    return empty ;
}
// =========================================================================
auto StateHolder::clear() -> void {
    auto empty = std::queue<ConnectionState>() ;
    {
        auto lock = std::lock_guard(access) ;
        std::swap(empty,states) ;
    }
}

// =========================================================================
// NameHolder
// =========================================================================

// =========================================================================
const std::string NameHolder::unknownKey = "UknownClient"s ;

// =========================================================================
auto NameHolder::add(const std::string &identity, const std::string &name) -> void {
    name_mapping.insert_or_assign(identity,name) ;
}

// =========================================================================
auto NameHolder::clear()-> void {
    name_mapping.clear() ;
}
// =========================================================================
auto NameHolder::operator[](const std::string &key) const -> const std::string& {
    auto iter = name_mapping.find(key) ;
    if (iter != name_mapping.end()) {
        return iter->second ;
    }
    return unknownKey ;
}

// =========================================================================
auto NameHolder::remove(const std::string &identity) -> void {
    auto iter = name_mapping.find(identity) ;
    if (iter != name_mapping.end()) {
        name_mapping.erase(iter) ;
    }
    
}

// =========================================================================
// ConnectionHolder
// =========================================================================

// =================================================================================================
auto ConnectionHolder::processPacket(Packet packet, ConnectionPointer connection) -> bool {
    switch (packet.packetID()) {
        case PacketType::IDENT: {
             nameMapping.add(connection->peer(),static_cast<IdentPacket*>(&packet)->handle() );
            logConnection(connection, true);
            break;
        }
        case PacketType::NOP: {
            auto ptr = static_cast<NopPacket*>(&packet) ;
            if (ptr->respond()) {
                auto response = NopPacket() ;
                connection->send(response);
            }
            break;
        }
        case PacketType::MYERROR:{
            auto ptr = static_cast<ErrorPacket*>(&packet) ;
            logError(connection, ptr) ;
            break;
        }
        default:
            break;
    }
    return true ;
}
// =================================================================================================
auto ConnectionHolder::processClose(ConnectionPointer connection) -> void {
    auto name = nameMapping[connection->peer()] ;
    
    logConnection(connection, false) ;
    nameMapping.remove(connection->peer()) ;
    connection->clearPeer() ;
    connection->setCloseCallback(nullptr) ;
    connection->setPacketRoutine(nullptr) ;
}

// =================================================================================================
auto ConnectionHolder::logConnection(ConnectionPointer &connection, bool state)-> void {

    auto name = nameMapping[connection->peer()] ;
    auto statename = "Connected"s;
    if (!state) {
        nameMapping.remove(connection->peer()) ;
        statename = "Disconnected"s;
    }
    auto msg = name + " = "s + util::sysTimeToString(connection->time()) + " , "s + statename + " , "s + connection->peer();
    DBGMSG(std::cout , msg);
    auto output = std::ofstream(connectionLog,std::ios::app);
    if (output.is_open()) {
        output << msg << std::endl;
    }

}

// =================================================================================================
auto ConnectionHolder::logError(ConnectionPointer &connection, ErrorPacket *ptr)-> void {
    auto name = nameMapping[connection->peer()] ;
  
    auto category = ptr->category() == ErrorPacket::AUDIO? "AUDIO":"PLAY" ;
    auto message = name + " = "s + util::sysTimeToString(util::ourclock::now())+" , " + category ;
    if (ptr->category() == ErrorPacket::PLAY) {
        message = message + " , "s +  ptr->name() ;
    }
    auto output = std::ofstream(errorLog,std::ios::app);
    if (output.is_open()) {
        output << message << std::endl;
    }

}

// =================================================================================================
ConnectionHolder::ConnectionHolder() { }

// =================================================================================================
ConnectionHolder::~ConnectionHolder() {
    this->clear() ;
}

// =================================================================================================
auto ConnectionHolder::setLogs(const std::string &connection_log, const std::string &errorlog) -> void {
    this->connectionLog = connection_log ;
    this->errorLog = errorlog ;
}

// =================================================================================================
auto ConnectionHolder::addConnection(ConnectionPointer pointer) -> void {
    pointer->timestamp();
    pointer->setPeer() ;
    {
        auto lock = std::lock_guard(access) ;
        connections.push_back(pointer) ;
    }
    pointer->setCloseCallback(std::bind(&ConnectionHolder::processClose,this,std::placeholders::_1));
    pointer->setPacketRoutine(std::bind(&ConnectionHolder::processPacket,this,std::placeholders::_1,std::placeholders::_2));
    // Now issue the read
    pointer->read() ;
}

// ===============================================================================================
auto ConnectionHolder::sendPacket(const Packet &packet) -> void {
    lastWrite = util::ourclock::now() ;
    auto lock = std::lock_guard(access) ;
    
    for (auto iter = connections.begin();iter != connections.end();) {
        if ( !(*iter)->is_open()) {
            nameMapping.remove((*iter)->peer());
            (*iter)->close() ;
            iter = connections.erase(iter);
        }
        else {
            (*iter)->send(packet) ;
            iter++ ;
        }
    }
    
}

// ===============================================================================================
auto ConnectionHolder::sendSync(FrameValue &value) -> void {
    lastWrite = util::ourclock::now() ;
    auto lock = std::lock_guard(access) ;
    for (auto iter = connections.begin();iter != connections.end();) {
        if ( !(*iter)->is_open()) {
            nameMapping.remove((*iter)->peer());
            (*iter)->close() ;
            iter = connections.erase(iter);
        }
        else {
            auto  packet = SyncPacket(value.value()) ;
            (*iter)->send(packet) ;
            iter++ ;
        }
    }
}

// ===============================================================================================
auto ConnectionHolder::sendLoad(const std::string& music, const std::string &light) -> void {
    auto packet = LoadPacket(music, light) ;
    this->sendPacket(packet);
}

// ===============================================================================================
auto ConnectionHolder::sendPlay(bool state, const FrameValue &value ) -> void {
    lastWrite = util::ourclock::now() ;
    auto packet = PlayPacket(state) ;
    for (auto iter = connections.begin();iter != connections.end();) {
        if ( !(*iter)->is_open()) {
            nameMapping.remove((*iter)->peer());
            (*iter)->close() ;
            iter = connections.erase(iter);
        }
        else {
            packet.setFrame(value.value());
            (*iter)->send(packet) ;
            iter++ ;
        }
    }
}

// ===============================================================================================
auto ConnectionHolder::sendNop(bool respond) -> void {
    auto packet = NopPacket(respond) ;
    this->sendPacket(packet);
}

// ===============================================================================================
auto ConnectionHolder::size() const  -> size_t {
    auto lock = std::lock_guard(access) ;
    return connections.size() ;
}

// ===============================================================================================
auto ConnectionHolder::clear() -> void {
    auto lock = std::lock_guard(access) ;
    for (auto iter = connections.begin() ; iter != connections.end();) {
        (*iter)->setCloseCallback(nullptr) ;
        (*iter)-> setPacketRoutine(nullptr) ;
        logConnection((*iter), false) ;
        (*iter)->close() ;
        iter = connections.erase(iter) ;
    }
}

// ===============================================================================================
auto ConnectionHolder::timeStamp() -> void {
    lastWrite = util::ourclock::now();
}
// ===============================================================================================
auto ConnectionHolder::refreshNop(int seconds) -> void {
    auto delta = std::chrono::duration_cast<std::chrono::seconds>(util::ourclock::now() - lastWrite).count() ;
    if (delta > seconds) {
        DBGMSG(std::cout, "Send a nop");

        sendNop(false);
    }
}
