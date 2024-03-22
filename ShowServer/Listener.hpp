// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Listener_hpp
#define Listener_hpp

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "asio.hpp"

#include "ConnectionHolder.hpp"

class Connection ;
class Show;
class Listener {
    asio::io_context connection_context ;
    asio::ip::tcp::acceptor acceptor;
    asio::io_context client_context ;
    
    std::thread connection_thread ;
    std::thread client_thread ;
    
    auto runConnections() -> void ;
    auto runClients() -> void ;
    
    ConnectionHolderPointer connectionHolder ;
    std::shared_ptr<Show> showPtr ;
    auto handleConnect(std::shared_ptr<Connection> client ,const asio::error_code& ec) -> void ;

public:
    Listener(ConnectionHolderPointer holder,std::shared_ptr<Show> showPtr) ;
    ~Listener();
    auto netSocket() -> asio::ip::tcp::acceptor & ;
    auto listen(std::uint16_t port) -> bool ;
    bool isListening;
    auto close() -> void ;
};
#endif /* Listener_hpp */
