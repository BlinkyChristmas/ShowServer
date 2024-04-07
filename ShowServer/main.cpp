// 

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <thread>

#include "packets/allpackets.hpp"

#include "ServerConfiguration.hpp"
#include "ClientHolder.hpp"
#include "Listener.hpp"
#include "ServerClient.hpp"
#include "StateHolder.hpp"
#include "Logger.hpp"

using namespace std::string_literals ;

auto runLoop(ServerConfiguration &config) -> bool ;

int main(int argc, const char * argv[]) {
    ServerConfiguration configuration ;
    auto exitcode = EXIT_SUCCESS ;
    try {
        if (argc < 2) { throw std::runtime_error("Missing configuration file!");}
        if (!configuration.load(std::filesystem::path(argv[1]))){
            throw std::runtime_error("Unable to process: "s + argv[1]);
        }
        // We are now have our configuration file, lets start our run loop
        if(!runLoop(configuration)) {
            throw std::runtime_error("Error occurred while running!") ;
        }
        
    }
    catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        exitcode = EXIT_FAILURE ;
    }
    catch(...) {
        std::cerr << "Unknown error!" << std::endl;
        exitcode = EXIT_FAILURE ;
    }
    return exitcode;
}

// ===========================================================================
// Our main run loop
// ==========================================================================
auto processClose(ClientPointer client) -> void ;
auto processIdentification(ClientPointer client, PacketPointer packet) -> bool ;
auto processError(ClientPointer client, PacketPointer packet) -> bool ;
auto initialConnect(ConnectionPointer connection) -> void ;

// These are the contexts that we need to service our server clients
asio::io_context client_context ;
asio::executor_work_guard<asio::io_context::executor_type> clientguard{asio::make_work_guard(client_context)} ;
std::thread threadClient;

auto runClient() -> void {
    client_context.run();
}

// =========================================================================================================
// The  globals that are used by the callbacks and runLoop
auto states = StateHolder() ;
auto clients = std::make_shared<ClientHolder>() ;
Logger logger ;

PacketRoutines routines ;

// ====================================================================
auto runLoop(ServerConfiguration &config) -> bool {
    threadClient = std::thread(&runClient) ;
    states.setLogFile(config.serverlog) ;
    states.log(config.serverlog ) ;
    logger.setLogFiles(config.connectlog, config.errorlog);
    routines.insert_or_assign(PacketType::IDENT,std::bind(&processIdentification,std::placeholders::_1,std::placeholders::_2)) ;
    routines.insert_or_assign(PacketType::MYERROR,std::bind(&processError,std::placeholders::_1,std::placeholders::_2)) ;

    auto listener = std::make_shared<Listener>(client_context) ;

    listener->setIntialConnect(std::bind(&initialConnect,std::placeholders::_1)) ;
    
    while (config.runSpan.inRange()) {
        try {
            if (config.refresh()) {
                // We shoud set anything we need to because the config file changed
                states.setLogFile(config.serverlog);
                logger.setLogFiles(config.connectlog, config.errorlog);
            }
        }
        catch(...) {
            // We had an error processing the config file, but we aren't going to worry about it, we did it initially, we will assume we can continue
        }
        states.setState(StateHolder::State::running, true) ;
        
        if (config.listenTime.inRange()) {
            // We should be listening!
            if (!listener->is_open()) {
                // we are not listening, so lets listen
                if (listener->listen(config.port)) {
                    // We started up!
                    states.setState(StateHolder::State::listening, true) ;
                }
            }
            
        }
        else {
            if (listener->is_open()) {
                // We should not be listening
                listener->close() ;
                states.setState(StateHolder::State::listening, false) ;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (!client_context.stopped()) {
        client_context.stop();
    }
    if (threadClient.joinable()){
        threadClient.join();
    }
    return true ;
}

// ===========================================================================
// Our callbacks
// ==========================================================================

// ==========================================================================
auto processClose(ClientPointer client) -> void {
    // we should log here
    logger.logConnection(client->handle(), client->ip(), false, client->timeStamp());
}

// ==========================================================================
auto processIdentification(ClientPointer client, PacketPointer packet) -> bool {
    auto ptr = static_cast<const IdentPacket*>(packet.get()) ;
    
    client->setHandle(ptr->handle()) ;
    // and we should log here!
    logger.logConnection(ptr->handle(), client->ip(), true, client->timeStamp());
    return true ;
}

// ==========================================================================
auto processError(ClientPointer client, PacketPointer packet) -> bool {
    
    return true ;
}

// ==========================================================================
auto initialConnect(ConnectionPointer connection) -> void {
    auto client = std::make_shared<ServerClient>(connection, routines) ;
    // add our close callback
    client->setRemoteClose(std::bind(&processClose,std::placeholders::_1)) ;
    // We need to check the state for show, and if playing!
    if (states.stateFor(StateHolder::State::startrange)){
        auto packet = ShowPacket(true) ;
        client->send(packet) ;
    }
    clients->add(client) ;
    client->read() ;
}
