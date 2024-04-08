//

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <thread>

#include "packets/allpackets.hpp"
#include "utility/dbgutil.hpp"
#include "utility/strutil.hpp"

#include "ServerConfiguration.hpp"
#include "ClientHolder.hpp"
#include "Listener.hpp"
#include "ServerClient.hpp"
#include "StateHolder.hpp"
#include "Logger.hpp"
#include "PlayList.hpp"
#include "Show.hpp"

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

auto songStopped() -> void ;
auto songSync(const FrameValue &value) -> void ;

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

Show ourShow;

// ====================================================================
auto runLoop(ServerConfiguration &config) -> bool {
    
    // Our initial setup
    threadClient = std::thread(&runClient) ;
    states.setLogFile(config.serverlog) ;
    states.log(config.serverlog ) ;
    ourShow.setFrameSync(config.frameupdate);
    ourShow.setSongStopped(std::bind(&songStopped));
    ourShow.setSyncCallback(std::bind(&songSync,std::placeholders::_1)) ;
    logger.setLogFiles(config.connectlog, config.errorlog);
    routines.insert_or_assign(PacketType::IDENT,std::bind(&processIdentification,std::placeholders::_1,std::placeholders::_2)) ;
    routines.insert_or_assign(PacketType::MYERROR,std::bind(&processError,std::placeholders::_1,std::placeholders::_2)) ;
    
    auto listener = std::make_shared<Listener>(client_context) ;
    
    listener->setIntialConnect(std::bind(&initialConnect,std::placeholders::_1)) ;
    
    // Our basic loop
    while (config.runSpan.inRange()) {
        try {
            if (config.refresh()) {
                // We shoud set anything we need to because the config file changed
                states.setLogFile(config.serverlog);
                logger.setLogFiles(config.connectlog, config.errorlog);
                ourShow.setFrameSync(config.frameupdate) ;
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
            if (listener->is_open()){
                // We should do our main stuff
                if (config.activeRange.inRange()){
                    if (!states.stateFor(StateHolder::State::activerange) ) {
                        clients->sendShow(true);
                        states.setState(StateHolder::State::activerange, true) ;
                    }
                }
                else {
                    if (states.stateFor(StateHolder::State::activerange) ) {
                        
                        states.setState(StateHolder::State::activerange, false) ;
                    }
                    
                }
                
                if (config.showTime.inRange()){
                    if (!states.stateFor(StateHolder::State::inshow)) {
                        states.setState(StateHolder::State::inshow, true) ;
                        clients->sendShow(true) ;
                        ourShow.reset() ;
                        ourShow.load(config.playlist);
                        
                    }
                    if (!ourShow.isPlaying) {
                        ourShow.shouldEndShow = !config.showTime.inRange();
                        auto entry = ourShow.nextEntry() ;
                        if (entry.valid()) {
                            DBGMSG(std::cout,util::format("Load: %s, %s",entry.musicName.c_str(),entry.lightName.c_str()));
                            clients->sendLoad(entry.musicName, entry.lightName) ;
                            std::this_thread::sleep_for(std::chrono::seconds(2)) ;
                            states.setState(StateHolder::State::playing, true) ;
                            ourShow.start(entry) ;
                            clients->sendPlay(true, FrameValue(0));
                        }
                        
                    }
                }
                else {
                    
                    if (!ourShow.isPlaying && ourShow.inShow) {
                        ourShow.shouldEndShow = !config.showTime.inRange();
                        
                        auto entry = ourShow.nextEntry() ;
                        if (entry.valid()) {
                            clients->sendLoad(entry.musicName, entry.lightName) ;
                            std::this_thread::sleep_for(std::chrono::seconds(2)) ;
                            states.setState(StateHolder::State::playing, true) ;
                            ourShow.start(entry) ;
                            clients->sendPlay(true, FrameValue(0));
                        }
                        else {
                            // If it isnt' valid!
                            // We need to set our show to "not be in show
                            ourShow.inShow = false ;
                            
                        }
                    }
                    if (!ourShow.inShow){
                        if (states.stateFor(StateHolder::State::inshow)) {
                            states.setState(StateHolder::State::inshow, false) ;
                            clients->sendShow(false);
                            ourShow.reset() ;
                        }
                    }
                }
            }
            clients->checkRefresh(80) ;
        }
        else {
            if (listener->is_open()) {
                // We should not be listening
                listener->close() ;
                states.setState(StateHolder::State::listening, false) ;
                // We are going to just shut everything down
                clients->clear() ;
            }
            if (ourShow.isPlaying) {
                ourShow.stop() ;
            }
            if (ourShow.inShow) {
                ourShow.inShow  = false ;
                ourShow.reset() ;
            }
            // Now, do our states
            if (states.stateFor(StateHolder::State::inshow)){
                states.setState(StateHolder::State::inshow, false);
            }
            if (states.stateFor(StateHolder::State::activerange)){
                states.setState(StateHolder::State::activerange, false);
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
    DBGMSG(std::cout, "Error reported by: "s+client->handle());
    return true ;
}

// ==========================================================================
auto initialConnect(ConnectionPointer connection) -> void {
    auto client = std::make_shared<ServerClient>(connection, routines) ;
    // add our close callback
    client->setRemoteClose(std::bind(&processClose,std::placeholders::_1)) ;
    // We need to check the state for show, and if playing!
    if (states.stateFor(StateHolder::State::activerange)){
        auto packet = ShowPacket(true) ;
        client->send(packet) ;
    }
    if (ourShow.isPlaying) {
        auto current = ourShow.currentEntry ;
        auto frame = ourShow.getCurrentFrame() ;
        if ( (current.framecount - frame.value()) > (3*270) ){
            auto load = LoadPacket(current.musicName,current.lightName);
            client->send(load);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            auto play = PlayPacket(true,frame.value()) ;
            client->send(play);
        }
    }
    clients->add(client) ;
    client->read() ;
}

// ==========================================================================
auto songStopped() -> void {
    clients->sendPlay(false,  0) ;
    states.setState(StateHolder::State::playing, false);
}
// ==========================================================================
auto songSync(const FrameValue &value) -> void {
    clients->sendSync(value);
    //DBGMSG(std::cout, "Sending sync: "s+std::to_string(value.value()));
}
