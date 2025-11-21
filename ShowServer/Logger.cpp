// Copyright © 2024 Charles Kerr. All rights reserved.

#include "Logger.hpp"
#include <fstream>
#include "utility/strutil.hpp"
#include "utility/timeutil.hpp"
#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

// =============================================================================================
auto Logger::setLogFiles(const std::string &connectionFile, const std::string &errorFile) -> void {
    connection_file = connectionFile ;
    error_file = errorFile ;
}

// =============================================================================================
auto Logger::logConnection(const std::string &handle, const std::string &ip, bool state, const util::ourclock::time_point &timepoint) -> void {
#if STANDALONE != 1
    static const std::string connection_format = "%s = %s, %s, %s"s;
    auto time = util::sysTimeToString(timepoint);
    if (!connection_file.empty()) {
        auto msg = util::format(connection_format, handle.c_str(),  time.c_str(),  (state?" Connected  ":"Disconnected"), ip.c_str()  );
        DBGMSG(std::cout, msg) ;
        auto output = std::ofstream(connection_file,std::ios::app) ;
        if (!output.is_open()) {
            return ;
        }
        output << msg << std::endl;
        output.close() ;
        
    }
#endif
}

// =============================================================================================
auto Logger::logError(const std::string &handle, const std::string &ip, const std::string &type) -> void {
    static const std::string error_format = "%s = %s, %s, %s"s;
    auto time = util::sysTimeToString(util::ourclock::now());
#if !defined(STANDALONE)
   if (!error_file.empty()) {
        auto msg = util::format(error_format, handle.c_str(), type.c_str(), ip.c_str() , time.c_str()  );
        DBGMSG(std::cout, "Error: "s + msg) ;
        auto output = std::ofstream(error_file,std::ios::app) ;
        if (!output.is_open()) {
            return ;
        }
        output << msg << std::endl;
        output.close() ;
    }
#endif
}

