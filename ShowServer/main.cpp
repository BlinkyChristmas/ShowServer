// 

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <functional>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>

#include "utility/timeutil.hpp"
#include "utility/strutil.hpp"

#include "ConfigFile.hpp"

using namespace std::string_literals ;

auto runLoop(ConfigFile &configuration,std::filesystem::path &config_file ) -> void ;

int main(int argc, const char * argv[]) {
    auto exitcode = EXIT_SUCCESS;
    try {
        if (argc < 2) {
            throw std::runtime_error("Configuration file now supplied as an argument");
        }
        auto config_file = std::filesystem::path(argv[1]) ;
        if (!std::filesystem::exists(config_file)) {
            throw std::runtime_error("Unable to locate: "s + config_file.string());
        }
        auto configuration = ConfigFile() ;
        if (!configuration.load(config_file)) {
            throw std::runtime_error("Unable to load: "s + config_file.string());
        }
        runLoop(configuration,config_file) ;
    }
    catch(const std::exception &e){
        std::cerr << e.what() << std::endl;
        exitcode = EXIT_FAILURE;
    }
    catch(...) {
        std::cerr << "Uknown Error, unknown exception!" << std::endl;
        exitcode = EXIT_FAILURE;

    }
    return exitcode;
}

// =====================================================================
// The run loop for the program!
// =====================================================================
#include "ConnectionHolder.hpp"
#include "Listener.hpp"
#include "Show.hpp"
#include "utility/dbgutil.hpp"
auto runLoop(ConfigFile &config,std::filesystem::path &config_file) -> void {
    {
        auto output = std::ofstream(config.connectlog, std::ios::app);
        if (output.is_open()) {
            output << "# Opened: " << util::sysTimeToString(util::ourclock::now()) << std::endl;
            output.close();
        }
    }
    {
        auto output = std::ofstream(config.errorlog, std::ios::app);
        if (output.is_open()) {
            output << "# Opened: " << util::sysTimeToString(util::ourclock::now()) << std::endl;
            output.close();
        }
    }

    auto connections = std::make_shared<ConnectionHolder>() ;
    connections->setLogs(config.connectlog, config.errorlog);
    auto show = std::make_shared<Show>(connections);
    show->setSyncInterval(config.frameupdate) ;
    auto listener = Listener(connections,show) ;
    bool inRunRange = false ;
    bool inStartRange = false ;
    bool inShow = false ;
    bool isListening = false ;
    bool songPlaying = false ;
    try{
        while (config.runTime.inRange()) {
            if (!inRunRange) {
                DBGMSG(std::cerr, "Setting run to on");

                inRunRange = true ;
                auto output = std::ofstream(config.serverlog, std::ios::app) ;
                if (output.is_open()) {
                    auto msg = "STATE = ON, OFF, OFF, "s + util::sysTimeToString(util::ourclock::now()) ;
                    output << msg << std::endl;
                    output.close() ;
                }
            }
            if (config.hasBeenUpdated(config_file)) {
                if (!config.load(config_file)) {
                    throw std::runtime_error("Error loading: "s + config_file.string());
                }
                connections->setLogs(config.connectlog, config.errorlog);
                show->setSyncInterval(config.frameupdate) ;
            }
            if (config.listenTime.inRange() && !listener.isListening) {
                // We need to check if our listener is connected?
                connections->clear() ;
                if (!listener.listen(config.port)){
                    DBGMSG(std::cerr, "Unsuccesful in starting listener on port: "s + std::to_string(config.port));
                }
                else {
                    DBGMSG(std::cout, "Listening");
                    connections->timeStamp();
                    inShow = false ;
                    isListening = true ;
                    // we need to log to the server log !
                    auto output = std::ofstream(config.serverlog,std::ios::app) ;
                    
                    if (output.is_open()) {
                        auto msg = "STATE = ON, ON, OFF, "s + util::sysTimeToString(util::ourclock::now()) ;
                        output << msg << std::endl;
                        output.close() ;
                    }
               }
            }
            else if (inShow && !config.listenTime.inRange()){
                if(show->isPlaying && !show->shouldStop) {
                    show->shouldStop = true ;
                }
            }
            if (listener.isListening){
                
                if (config.startRange.inRange() && !inStartRange) {
                    // We need to send out our show start messge
                    inStartRange = true ;
                    DBGMSG(std::cout, "Send Show Start");

                    auto packet = ShowPacket(true) ;
                    connections->sendPacket(packet) ;

                }
                if (config.showTime.inRange()) {
                    if (!show->showPlaying) {
                        DBGMSG(std::cout, "Actually Start the Show");
                        // we need to load the show, and "start the show"
                        // load the playlist
                        auto playlist = PlayList(config.playlist) ;
                        inShow = true ;
                        show->setPlayList(playlist) ;
                        show->start() ;
                        // We should log the server log!
                        auto output = std::ofstream(config.serverlog,std::ios::app) ;
                        
                        if (output.is_open()) {
                            auto msg = "STATE = ON, ON, ON, "s + util::sysTimeToString(util::ourclock::now()) ;
                            output << msg << std::endl;
                            output.close() ;
                        }

                    }
                }
                else if (show->showPlaying && !show->shouldStop){
                    DBGMSG(std::cout, "Tell the show it should stop");

                    show->shouldStop = true ;
                }
                if (show->showPlaying) {
                    if (!show->isPlaying) {
                        if (show->loadNext()) {
                            // we can pause for a bit
                            std::this_thread::sleep_for(std::chrono::seconds(2));
                            show->play() ;
                        }
                        else {
                            // We are finished!
                            show->showPlaying = false ;
                        }
                    }
                }
                else if (inShow) {
                    DBGMSG(std::cout, "Send Show Stop");

                    // we need to send a show stop
                    inShow = false ;
                    inStartRange = false ;
                    songPlaying = false;
                    auto packet = ShowPacket(false) ;
                    connections->sendPacket(packet) ;
                    // We should log the server log!
                    auto output = std::ofstream(config.serverlog,std::ios::app) ;
                    
                    if (output.is_open()) {
                        auto msg = "STATE = ON, ON, OFF, "s + util::sysTimeToString(util::ourclock::now()) ;
                        output << msg << std::endl;
                        output.close() ;
                    }

                    // Log the server log!
                }
                
                if (!inShow) {
                    connections->refreshNop(80) ;
                }
            }
            if (!config.listenTime.inRange() && listener.isListening && !inShow){
                // we need to stop listening!
                DBGMSG(std::cout, "Stop Listening");
                listener.netSocket().close() ;
                isListening = false ;
                // Now close our connections that we may have running
                connections->clear() ;
                std::this_thread::sleep_for(std::chrono::seconds(1)) ; // Allow things to settle
                // Log to server log!
                listener.close() ;
                auto output = std::ofstream(config.serverlog, std::ios::app) ;
                auto msg = "STATE = "s +"ON,"s + ",OFF,"s  + "OFF,"s + util::sysTimeToString(util::ourclock::now()) ;
                if (output.is_open()) {
                    output << msg << std::endl;
                    output.close();
                }
                
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        // we are outside the run time, we need to ensure we are not listinging or playing
        while (show->showPlaying) {
            if (show->shouldStop != false) {
                show->shouldStop = true ;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        // We are shutting down for good!
        if (listener.isListening) {
            listener.netSocket().close() ;
            connections->clear() ;
            std::this_thread::sleep_for(std::chrono::seconds(1)) ; // Allow things to settle
            listener.close() ;
        }
        
    }
    catch(...) {
        throw ;
    }
    auto output = std::ofstream(config.serverlog,std::ios::app) ;
    
    if (output.is_open()) {
        auto msg = "STATE = OFF, OFF, OFF, "s + util::sysTimeToString(util::ourclock::now()) ;
        output << msg << std::endl;
        output.close() ;
    }

}
