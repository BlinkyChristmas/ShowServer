// Copyright © 2024 Charles Kerr. All rights reserved.

#include "ConnectionHolder.hpp"

#include <functional>
#include <algorithm>
#include <fstream>

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;


// =========================================================================
// ConnectionHolder
// =========================================================================

// =================================================================================================
auto ConnectionHolder::processPacket(Packet packet, ConnectionPointer connection) -> bool {
    switch (packet.packetID()) {
        case PacketType::IDENT: {
            connection->handle = static_cast<IdentPacket*>(&packet)->handle() ;
            auto output = std::ofstream(connectionLog,std::ios::app);
            if (output.is_open()) {
                connection->log(output,true) ;
            }
            output.close() ;
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
    auto output = std::ofstream(connectionLog,std::ios::app);
    if (output.is_open()) {
        connection->log(output,false) ;
    }
    output.close() ;
    connection->clearPeer() ;
}


// =================================================================================================
auto ConnectionHolder::logError(ConnectionPointer &connection, ErrorPacket *ptr)-> void {
    auto category = ptr->category() == ErrorPacket::AUDIO? "AUDIO":"PLAY" ;
    auto message = connection->handle + " = "s + util::sysTimeToString(util::ourclock::now())+" , " + category ;
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
        if ( (*iter)->is_open()) {
            // He hasn't been logged yet
            (*iter)->close(connectionLog) ;
        }
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
