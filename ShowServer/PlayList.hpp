//Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef PlayList_hpp
#define PlayList_hpp

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

//======================================================================
//======================================================================
// ListEntry
//======================================================================
struct ListEntry {
    std::string musicName ;
    std::string lightName ;
    std::uint32_t framecount ;
    
    ListEntry() ;
    ListEntry(const std::string &line) ;
    auto valid() const -> bool ;
   
};
//======================================================================
// PlayList
//======================================================================
struct PlayList {
    std::vector<ListEntry> prologue ;
    std::vector<ListEntry> body ;
    std::vector<ListEntry> epilogue ;
    
    PlayList() = default ;
    PlayList(const std::filesystem::path &path) ;
    
    auto load(const std::filesystem::path &path) -> bool ;
    auto prologueCount() const -> std::int32_t ;
    auto bodyCount() const -> std::int32_t ;
    auto epilogueCount() const -> std::int32_t ;
    
    auto size() const -> size_t ;

};


#endif /* PlayList_hpp */
