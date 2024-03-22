// Copyright © 2024 Charles Kerr. All rights reserved.

#include "Listener.hpp"
#include "ConnectionHolder.hpp"
#include <functional>

#include "utility/dbgutil.hpp"
#include "Show.hpp"

using namespace std::string_literals ;
// ===================================================================
auto Listener::runConnections() -> void {
    DBGMSG(std::cout, "Run connection contex: ON");
    isListening = true ;
    connection_context.run() ;
    isListening = false ;
    DBGMSG(std::cout, "Run connection contex: OFF");
}

// ===================================================================
auto Listener::runClients() -> void {
    DBGMSG(std::cout, "Run client contex: ON");

    client_context.run() ;
    DBGMSG(std::cout, "Run client contex: OFF");
}

// ===================================================================
Listener::Listener(ConnectionHolderPointer holder, std::shared_ptr<Show> showPtr ):acceptor(connection_context),isListening(false) {
    connectionHolder = holder ;
    this->showPtr = showPtr;
}

// ===================================================================
Listener::~Listener() {
    if (acceptor.is_open()) {
        acceptor.close() ;
    }
    connectionHolder->clear();
    if (!client_context.stopped()){
        client_context.stop();
    }
    if (!connection_context.stopped()){
        connection_context.stop();
    }
    if (client_thread.joinable()) {
        client_thread.join();
    }
    if (connection_thread.joinable()){
        connection_thread.join();
    }
}

// ===================================================================
auto Listener::handleConnect(std::shared_ptr<Connection> client ,const asio::error_code& ec) -> void  {
    if (ec) {
        DBGMSG(std::cerr,"Error on connection: "s + ec.message());
        return ;
    }
    
    // Do something with this connection
    client->setPeer();
    client->timestamp() ;
    // is our thread running?
    auto restartThread = false ;
    if (connectionHolder->size() < 1 || client_context.stopped()) {
        // This means we probably it just got the one we added
        if (client_context.stopped()) {
            client_context.restart();
        }
        if (client_thread.joinable()) {
            client_thread.join();
            client_thread = std::thread();
         }
        restartThread = true ;

    }
    connectionHolder->addConnection(client) ;
    DBGMSG(std::cout, "After adding this connection, there are: "s + std::to_string(connectionHolder->size()));
    // Read on the client should have been done by the addConnection call
    if (restartThread) {
        client_thread = std::thread(&Listener::runClients,this) ;
    }

    if (showPtr->showPlaying) {
        auto showpacket = ShowPacket(true) ;
        DBGMSG(std::cout, "Sending a connecting client a show on");
        client->send(showpacket);
    }
    auto current = showPtr->current() ;
    if (current.isValid()){
        auto loadpacket = LoadPacket(current.music,current.light) ;
        DBGMSG(std::cout, "Sending a connecting client a load: "s + current.music+" , "s + current.light);
        client->send(loadpacket);
        if (showPtr->isPlaying) {
            auto frame = showPtr->currentFrame() ;
            auto playPacket = PlayPacket(true) ;
            playPacket.setFrame(frame.value());
            DBGMSG(std::cout, "Sending a connecting client a play on, frame: "s + std::to_string(playPacket.frame()));

            client->send(playPacket);
        }
    }
    
    
    // we will need a new client
    auto newclient = std::make_shared<Connection>(client_context) ;
    acceptor.async_accept(newclient->socket(),std::bind(&Listener::handleConnect,this,newclient,std::placeholders::_1)) ;
}

// ===================================================================
auto Listener::listen(std::uint16_t port) -> bool {
    if (isListening) {
        return false ;
    }
    if (connection_thread.joinable()) {
        connection_thread.join();
        connection_thread = std::thread() ;
    }
    DBGMSG(std::cout, "Starting listener on port: "s + std::to_string(port));
    asio::error_code ec ;
    if (acceptor.is_open()) {
        // The acceptor is open!
        return false ;
    }
    acceptor.open(asio::ip::tcp::v4(),ec) ;
    if (ec) {
        std::cerr << "Error opening acceptor"<< ec.message() << std::endl;
        return false ;
    }
    asio::socket_base::reuse_address option(true) ;
    acceptor.set_option(option) ;
    acceptor.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port ),ec) ;
    if (ec) {
        std::cerr << "Error binding acceptor: " << ec.message() << std::endl;
        acceptor.close() ;
        return false ;
    }
    acceptor.listen(50, ec) ;
    if (ec) {
        std::cerr << "Error listening: " << ec.message() << std::endl;
        acceptor.close() ;
        return false ;
    }
    auto newclient = std::make_shared<Connection>(client_context) ;
    connection_context.restart() ;
    acceptor.async_accept(newclient->socket() , std::bind(&Listener::handleConnect,this, newclient,std::placeholders::_1)) ;
    connection_thread = std::thread(&Listener::runConnections,this) ;
    return true ;
}

// ==========================================================================================
auto Listener::close() -> void {
    
    DBGMSG(std::cout, "Closing Listener");
    if (acceptor.is_open()) {
        acceptor.close() ;
    }
    if (connection_thread.joinable()) {
        connection_thread.join();
        connection_thread = std::thread() ;
    }
    connectionHolder->clear() ;
    if (client_thread.joinable()) {
        client_thread.join();
        client_thread = std::thread() ;
    }
}
