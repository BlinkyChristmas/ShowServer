// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef StateHolder_hpp
#define StateHolder_hpp

#include <string>
#include <vector>

class StateHolder{
    std::vector<bool> states ;
    std::string logfile ;
    static const std::vector<std::string> STATENAMES ;
public:
    enum State{
        running=0,listening,startrange,inshow,playing
    };
    StateHolder() ;
    StateHolder(const std::string &name) ;
    auto setLogFile(const std::string &name) -> void ;
    
    auto setState(State position, bool state) -> void ;
    auto stateFor(State position) const -> bool ;
    
    auto log(const std::string &logfile) -> bool ;
    auto describe() const -> std::string ;
};

#endif /* StateHolder_hpp */
