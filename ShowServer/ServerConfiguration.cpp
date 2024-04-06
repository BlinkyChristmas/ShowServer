// Copyright © 2024 Charles Kerr. All rights reserved.

#include "ServerConfiguration.hpp"
#include "utility/strutil.hpp"

using namespace std::string_literals ;
// ============================================================================
ServerConfiguration::ServerConfiguration():BaseConfiguration(){
    delay = 0 ;
    frameupdate = 270 ;
    port = 50000 ;
}

// ============================================================================
ServerConfiguration::ServerConfiguration(const std::filesystem::path &path):ServerConfiguration(){
    if (!this->load(path)) {
        throw std::runtime_error("Error processing: "s + path.string());
    }
}

// ============================================================================
auto ServerConfiguration::processKeyValue(const std::string &key, const std::string &value) -> void {
    
    auto ukey = util::upper(key) ;
    
    if (ukey == "PORT"){
       
        port = static_cast<std::uint16_t>(std::stoul(value,nullptr,0)) ;
    }
    else if (ukey == "RUNSPAN") {
        runSpan = util::MonthRange(value) ;
    }
    else if (ukey == "LISTEN") {
        listenTime = util::HourRange(value) ;
    }
    else if (ukey == "SHOW") {
        showTime = util::HourRange(value) ;
    }
    else if (ukey == "DELAY") {
        delay = std::stoi(value,nullptr,0) ;
    }
    else if (ukey == "PLAYLIST") {
        playlist = std::filesystem::path(value) ;
    }
    else if (ukey == "SYNCRATE") {
        frameupdate = std::stoi(value,nullptr,0);
    }
    else if (ukey == "ERRORLOG") {
        errorlog = value ;
    }
    else if (ukey == "CONNECTLOG") {
        connectlog = value ;
    }
    else if (ukey == "SERVERLOG") {
        serverlog = value ;
    }

}
