// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Listener_hpp
#define Listener_hpp

#include <string>
#include <memory>

#include "asio.hpp"

#include "ClientHolder.hpp"

class Listener ;

using ListenerPointer = std::shared_ptr<Listener> ;
using ConnectCallback = std::function<void(ClientPointer)> ;

class Listener : public std::enable_shared_from_this<Listener> {
    
    asio::io_context connection_context ;
    asio::executor_work_guard<asio::io_context::executor_type> connectionguard{asio::make_work_guard(connection_context)} ;
    
    asio::io_context client_context ;
    asio::executor_work_guard<asio::io_context::executor_type> clientguard{asio::make_work_guard(client_context)} ;

    std::thread threadConnection ;
    std::thread threadClient;

    auto runConnection() -> void ;
    auto runClient() -> void ;
    
    asio::ip::tcp::acceptor acceptor; 
    PacketRoutines routines ;
    
    ClientHolderPointer clients ;
    ConnectCallback initialConnect ;
public:
    Listener(ClientHolderPointer holder, const PacketRoutines & routines) ;
    ~Listener() ;
    auto setIntialConnect(ConnectCallback inital) -> void ;
};

#endif /* Listener_hpp */
