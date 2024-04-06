// Copyright © 2024 Charles Kerr. All rights reserved.

#include "StateHolder.hpp"

#include "utility/timeutil.hpp"
#include "utility/strutil.hpp"
#include <fstream>

using namespace std::string_literals ;


// ========================================================================
auto StateHolder::log(const std::string &logfile) -> bool {
    if (logfile.empty()){
        return false ;
    }
    auto msg = "STATE = "s + this->describe() + ", "s +  util::sysTimeToString(util::ourclock::now());

    auto output = std::ofstream(logfile,std::ios::app) ;
    if (!output.is_open()) {
        return false ;
    }
    output << msg<< std::endl;
    output.close() ;
    return true ;
}

// ========================================================================
StateHolder::StateHolder(){
    states = std::vector<bool>(5,false) ;
}
// ========================================================================
StateHolder::StateHolder(const std::string &name):StateHolder(){
    setLogFile(name) ;
}
// ======================================================================
auto StateHolder::setLogFile(const std::string &name) -> void {
    logfile = name ;
}

// ========================================================================
auto StateHolder::setState(State position, bool state) -> void {
    if (states[position] != state) {
        states[position] = state ;
        log(logfile) ;
    }
}

// ========================================================================
auto StateHolder::stateFor(State position) const -> bool {
    return states[position] ;
}
// ========================================================================
auto StateHolder::describe() const -> std::string {
    auto msg = ""s ;
    for (auto entry: states) {
        if (!msg.empty()){
            msg+= ", "s ;
        }
        msg += (entry?"ON"s:"OFF"s)  ;
    }
    return msg ;
}
