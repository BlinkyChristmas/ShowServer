// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef ServerConfiguration_hpp
#define ServerConfiguration_hpp

#include <cstdint>

#include "utility/BaseConfiguration.hpp"

#include "utility/timeutil.hpp"

class ServerConfiguration : public BaseConfiguration {
  
    auto processKeyValue(const std::string &key, const std::string &value) -> void final ;
public:
    
    util::HourRange showTime ;
    util::HourRange listenTime ;
    util::MonthRange runSpan ;
    util::HourRange startRange;
    std::string connectlog ;
    std::string errorlog ;
    std::string serverlog ;
    int delay ;
    std::uint16_t port ;
    std::int32_t frameupdate;
    std::filesystem::path playlist ;

    
    ServerConfiguration() ;
    ServerConfiguration(const std::filesystem::path &path) ;
    
    
};

#endif /* ServerConfiguration_hpp */
