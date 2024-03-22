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


// =========================================================================
struct ConnectionState {
    std::string identity ;
    bool state ;
    std::string timestamp ;
    std::string name ;
    
    ConnectionState(const std::string &connection_identity, const std::string &time_marker, const std::string &client_name,  bool connection_state);
    auto describe() const -> std::string ;
};

// =========================================================================
class StateHolder {
    mutable std::mutex access ;
    std::queue<ConnectionState> states ;
    auto add(ConnectionState &state) -> void ;
    auto retrieve() -> std::queue<ConnectionState> ;
    auto clear() -> void ;
};

// =========================================================================
struct NameHolder {
    std::unordered_map<std::string, std::string> name_mapping ;
    static const std::string unknownKey ;
    auto clear() -> void ;
    auto add(const std::string &identity, const std::string &name) -> void ;
    auto operator[](const std::string &key) const -> const std::string& ;
    auto remove(const std::string &identity) -> void ;
};
class Listener ;
class ConnectionHolder ;
using ConnectionHolderPointer = std::shared_ptr<ConnectionHolder>;
// =========================================================================
class ConnectionHolder {
    friend class Connection ;
    friend class Listener ;
    std::string connectionLog ;
    std::string errorLog;
    
    NameHolder nameMapping ;
    
    util::ourclock::time_point lastWrite ;
    std::deque<ConnectionPointer> connections ;
    mutable std::mutex access ;
    
    auto processPacket(Packet packet, ConnectionPointer connection) -> bool ;
    auto processClose(ConnectionPointer connection) -> void ;
    
    auto logConnection(ConnectionPointer &connection, bool state)-> void ;
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
