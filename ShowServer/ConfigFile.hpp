//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef ConfigFile_hpp
#define ConfigFile_hpp

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>

#include "utility/timeutil.hpp"

//======================================================================
struct ConfigFile {
    std::filesystem::path playlist ;
    
    util::HourRange showTime ;
    util::HourRange listenTime ;
    util::MonthRange runTime ;
    util::HourRange startRange;
    std::string connectlog ;
    std::string errorlog ;
    std::string serverlog ;
    int delay ;
    std::uint16_t port ;
    std::int32_t frameupdate;
    
    std::filesystem::file_time_type lastRead ;
    auto reset() -> void ;
    ConfigFile() ;
    auto load(const std::filesystem::path &path) -> bool ;
    auto hasBeenUpdated(const std::filesystem::path &path) const -> bool ;
    auto refresh(const std::filesystem::path &path) -> void ;
};

#endif /* ConfigFile_hpp */
