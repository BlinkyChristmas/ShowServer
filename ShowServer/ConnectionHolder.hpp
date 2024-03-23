// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef ConnectionHolder_hpp
#define ConnectionHolder_hpp

#include <chrono>
#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>

#include "utility/timeutil.hpp"
#include "packets/allpackets.hpp"

#include "network/Connection.hpp"
#include "network/FrameValue.hpp"




class Listener ;
class ConnectionHolder ;
using ConnectionHolderPointer = std::shared_ptr<ConnectionHolder>;
// =========================================================================
class ConnectionHolder {
    friend class Connection ;
    friend class Listener ;
    std::string connectionLog ;
    std::string errorLog;
    
    util::ourclock::time_point lastWrite ;
    std::deque<ConnectionPointer> connections ;
    mutable std::mutex access ;
    
    auto processPacket(Packet packet, ConnectionPointer connection) -> bool ;
    auto processClose(ConnectionPointer connection) -> void ;
    
    auto logError(ConnectionPointer &connection, ErrorPacket *error)-> void ;
    
public:
    ConnectionHolder() ;
    ~ConnectionHolder() ;
    auto setLogs(const std::string &connection_log, const std::string &errorlog) -> void ;
    auto addConnection(ConnectionPointer pointer) -> void ;
    
    auto sendPacket(const Packet &packet) -> void ;
    auto sendSync(FrameValue &value) -> void ;
    auto sendLoad(const std::string& music, const std::string &light) -> void ;
    auto sendPlay(bool state, const FrameValue &value = FrameValue()) -> void ;
    auto sendNop(bool respond) -> void ;
    
    auto size() const ->size_t ;
    auto clear() -> void ;
    auto timeStamp() -> void ;
    auto refreshNop(int seconds) -> void ;
} ;

#endif /* ConnectionHolder_hpp */
