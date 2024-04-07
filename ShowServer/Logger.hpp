// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Logger_hpp
#define Logger_hpp

#include <iostream>
#include <string>
#include "utility/timeutil.hpp"

class Logger {
    std::string connection_file ;
    std::string error_file ;
    
public:
    Logger() = default ;
    
    auto setLogFiles(const std::string &connectionFile, const std::string &errorFile) -> void ;
    
    auto logConnection(const std::string &handle, const std::string &ip, bool state, const util::ourclock::time_point &timepoint) -> void ;
    auto logError(const std::string &handle, const std::string &ip, const std::string &type) -> void ;
    
};
#endif /* Logger_hpp */
