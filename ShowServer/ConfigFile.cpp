//Copyright © 2024 Charles Kerr. All rights reserved.

#include "ConfigFile.hpp"

#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <vector>

#include "utility/strutil.hpp"
#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

//======================================================================
auto ConfigFile::reset() -> void {
    showTime = util::HourRange() ;
    listenTime  =  util::HourRange();
    runTime = util::MonthRange();
    delay = 0 ;
    port = 50000 ;
    lastRead =  std::chrono::file_clock::now() ;
    errorlog = "";
    connectlog = "" ;
    serverlog = "server.log"s ;
    playlist = std::filesystem::path() ;
}
// ==========================================================================================
ConfigFile::ConfigFile() : port(50000), delay(0), lastRead(std::chrono::file_clock::now()),frameupdate(270),serverlog("server.log") {
}
// ==========================================================================================
auto ConfigFile::load(const std::filesystem::path &path) -> bool {
    this->reset();
    auto input = std::ifstream(path.string()) ;
    if (!input.is_open()){
        return false ;
    }
    auto buffer = std::vector<char>(1024,0) ;
    while (input.good() && !input.eof()){
        input.getline(buffer.data(),buffer.size()-1) ;
        if (input.gcount()>0) {
            buffer[input.gcount()] = 0 ;
            std::string line = buffer.data();
            line = util::trim(util::strip(line,"#")) ;
            if (!line.empty()){
                auto [skey,value] = util::split(line,"=") ;
                auto key = util::upper(skey) ;
                try {
                    if (key == "PORT"){
                       
                        port = static_cast<std::uint16_t>(std::stoul(value,nullptr,0)) ;
                    }
                    else if (key == "RUNSPAN") {
                        runTime = util::MonthRange(value) ;
                    }
                    else if (key == "LISTEN") {
                        listenTime = util::HourRange(value) ;
                    }
                    else if (key == "SHOW") {
                        showTime = util::HourRange(value) ;
                    }
                    else if (key == "DELAY") {
                        delay = std::stoi(value,nullptr,0) ;
                    }
                    else if (key == "PLAYLIST") {
                        playlist = std::filesystem::path(value) ;
                    }
                    else if (key == "SYNCRATE") {
                        frameupdate = std::stoi(value,nullptr,0);
                    }
                    else if (key == "ERRORLOG") {
                        errorlog = value ;
                    }
                    else if (key == "CONNECTLOG") {
                        connectlog = value ;
                    }
                    else if (key == "SERVERLOG") {
                        serverlog = value ;
                    }
                }
                catch(...) {
                    DBGMSG(std::cerr, "Error parsing line: "s + line);
                }
            }
        }
    }
    lastRead = std::filesystem::last_write_time(path) ;
    startRange = util::HourRange((showTime.startTime - delay), showTime.endTime);
    return true ;
}
// ==========================================================================================
auto ConfigFile::hasBeenUpdated(const std::filesystem::path &path) const -> bool {
    auto lastwrite = std::filesystem::last_write_time(path) ;
    
    if ( std::chrono::duration_cast<std::chrono::seconds>(lastwrite - lastRead).count() > 0 ){
        return true ;
    }
    return false ;
}

// ==========================================================================================
auto ConfigFile::refresh(const std::filesystem::path &path) -> void {
    if (this->hasBeenUpdated(path)){
        if (!this->load(path)) {
            throw std::runtime_error("Unable to process configuration: "s + path.string() );
        }
    }
}
