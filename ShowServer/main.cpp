// 

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <thread>

#include "packets/IdentPacket.hpp"

#include "ServerConfiguration.hpp"
#include "ClientHolder.hpp"
#include "Listener.hpp"
#include "ServerClient.hpp"
#include "StateHolder.hpp"

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
auto initialConnect(ClientPointer client) -> void ;

auto states = StateHolder() ;
// ====================================================================
auto runLoop(ServerConfiguration &config) -> bool {
    states.setLogFile(config.serverlog) ;
    states.log(config.serverlog ) ;
    auto clients = std::make_shared<ClientHolder>() ;
    PacketRoutines routines ;
    routines.insert_or_assign(PacketType::IDENT,std::bind(&processIdentification,std::placeholders::_1,std::placeholders::_2)) ;
    
    auto listener = std::make_shared<Listener>(clients,routines) ;
    listener->setIntialConnect(std::bind(&initialConnect,std::placeholders::_1)) ;
    
    while (config.runSpan.inRange()) {
        try {
            if (config.refresh()) {
                // We shoud set anything we need to becasue the config file changed
                states.setLogFile(config.serverlog);
            }
        }
        catch(...) {
            // We had an error processing the config file, but we aren't going to worry about it, we did it initially, we will assume we can continue
        }
        states.setState(StateHolder::State::running, true) ;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return true ;
}

// ===========================================================================
// Our callbacks
// ==========================================================================

// ==========================================================================
auto processClose(ClientPointer client) -> void {
    // we should log here
}

// ==========================================================================
auto processIdentification(ClientPointer client, PacketPointer packet) -> bool {
    auto ptr = static_cast<const IdentPacket*>(packet.get()) ;
    
    client->setHandle(ptr->handle()) ;
    // and we should log here!
    
    return true ;
}

// ==========================================================================
auto initialConnect(ClientPointer client) -> void {
    
}
