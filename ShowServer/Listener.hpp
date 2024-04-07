// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Listener_hpp
#define Listener_hpp

#include <string>
#include <memory>

#include "asio.hpp"

#include "ClientHolder.hpp"

class Listener ;

using ListenerPointer = std::shared_ptr<Listener> ;
using ConnectCallback = std::function<void(ConnectionPointer)> ;

class Listener : public std::enable_shared_from_this<Listener> {
    
    asio::io_context connection_context ;
    asio::executor_work_guard<asio::io_context::executor_type> connectionguard{asio::make_work_guard(connection_context)} ;
    
    asio::io_context *client_context ;

    std::thread threadConnection ;

    auto runConnection() -> void ;
    asio::ip::tcp::acceptor acceptor; 
    
    ConnectCallback initialConnect ;
    
    auto handleConnect(std::shared_ptr<Connection> client ,const asio::error_code& ec) -> void ;
public:
    Listener(asio::io_context &client_context) ;
    ~Listener() ;
    auto setIntialConnect(ConnectCallback inital) -> void ;
    auto listen(std::uint16_t port) -> bool;
    auto is_open() const -> bool ;
    auto close() -> void ;
};

#endif /* Listener_hpp */
