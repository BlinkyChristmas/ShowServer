// Copyright © 2024 Charles Kerr. All rights reserved.

#include "Listener.hpp"

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

// ==============================================================================
auto Listener::runConnection() -> void {
    connection_context.run() ;
}


// ==============================================================================
Listener::Listener( asio::io_context &client_context) : acceptor(connection_context),initialConnect(nullptr){
    this->client_context = &client_context ;
    this->threadConnection = std::thread(&Listener::runConnection,this) ;
}

// ==============================================================================
Listener::~Listener() {
    // First, stop our connections
    if (acceptor.is_open()) {
        acceptor.close() ;
    }
    
    // now, stop our context
    if (!connection_context.stopped()){
        connection_context.stop();
    }
    
    // check if we have threads closing, and wait for them
    if (threadConnection.joinable()) {
        threadConnection.join() ;
    }
}

// ==============================================================================
auto Listener::setIntialConnect(ConnectCallback inital) -> void {
    initialConnect = inital ;
}

// ===================================================================
auto Listener::handleConnect(std::shared_ptr<Connection> client ,const asio::error_code& ec) -> void  {
    if (ec) {
        
        //DBGMSG(std::cerr,util::sysTimeToString(util::ourclock::now())+": "s + "Listener Error on connection: "s + ec.message());
        if (acceptor.is_open()) {
            try {
                acceptor.close();
            }
            catch(...) {}
        }
        return ;
    }

    // Do something with this connection
    client->setPeer();
    client->timestamp() ;
    if (initialConnect != nullptr){
        initialConnect(client) ;
    }
    // we will need a new client to listen for
    auto newclient = std::make_shared<Connection>(*client_context) ;

    acceptor.async_accept(newclient->socket(),std::bind(&Listener::handleConnect,this->shared_from_this(),newclient,std::placeholders::_1)) ;

}

// ===================================================================
auto Listener::listen(std::uint16_t port) -> bool {
    //DBGMSG(std::cout, util::sysTimeToString(util::ourclock::now())+": "s + "Starting listener on port: "s + std::to_string(port) );
    
    asio::error_code ec ;
    if (acceptor.is_open()) {
        // The acceptor is open!
        return false ;
    }
    acceptor.open(asio::ip::tcp::v4(),ec) ;
    if (ec) {
        DBGMSG(std::cerr,util::sysTimeToString(util::ourclock::now())+": "s + "Error opening acceptor"s + ec.message() );
        return false ;
    }
    asio::socket_base::reuse_address option(true) ;
    acceptor.set_option(option) ;
    acceptor.bind(asio::ip::tcp::endpoint(asio::ip::address_v4::any(), port ),ec) ;
    if (ec) {
        DBGMSG(std::cerr, util::sysTimeToString(util::ourclock::now())+": "s + "Error binding acceptor: "s + ec.message() );
        acceptor.close() ;
        return false ;
    }
    acceptor.listen(50, ec) ;
    if (ec) {
        DBGMSG(std::cerr,util::sysTimeToString(util::ourclock::now())+": "s + "Error listening: "s + ec.message() );
        acceptor.close() ;
        return false ;
    }
    auto newconnection = std::make_shared<Connection>(*client_context) ;
    acceptor.async_accept(newconnection->socket() , std::bind(&Listener::handleConnect,this->shared_from_this(), newconnection, std::placeholders::_1));
    return true ;
}

// ===================================================================
auto Listener::is_open() const -> bool {
    return acceptor.is_open();
}
// ===================================================================
auto Listener::close() -> void {
    if (acceptor.is_open()) {
        acceptor.close() ;
    }
}


