// Copyright © 2024 Charles Kerr. All rights reserved.

#include "Listener.hpp"

// ==============================================================================
auto Listener::runConnection() -> void {
    connection_context.run() ;
}

// ==============================================================================
auto Listener::runClient() -> void {
    client_context.run();
}

// ==============================================================================
Listener::Listener(ClientHolderPointer holder,const PacketRoutines & routines) : acceptor(connection_context),initialConnect(nullptr){
    clients = holder ;
    this->routines = routines ;
}

// ==============================================================================
Listener::~Listener() {
    // First, stop our connections
    if (acceptor.is_open()) {
        acceptor.close() ;
    }
    
    // Now, we should clear out any clients we have
    clients->clear() ;
    // now, stop our two contexts
    if (!connection_context.stopped()){
        connection_context.stop();
    }
    if (!client_context.stopped()){
        client_context.stop();
    }
    
    // check if we have threads closing, and wait for them
    if (threadConnection.joinable()) {
        threadConnection.join() ;
    }
    if (threadClient.joinable()) {
        threadClient.join() ;
    }
}

// ==============================================================================
auto Listener::setIntialConnect(ConnectCallback inital) -> void {
    initialConnect = inital ;
}
