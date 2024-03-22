// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Show_hpp
#define Show_hpp

#include <iostream>
#include <string>
#include <mutex>
#include <memory>

#include "PlayList.hpp"
#include "network/FrameTimer.hpp"
#include "ConnectionHolder.hpp"

struct CurrentEntry{
    std::string music;
    std::string light ;
    int framecount ;
    auto isValid() -> bool  ;
};

class Show{
    enum class ShowState {
        prologue,body,epilogue
    };
    ShowState state ;
    int nextEntry ;
    PlayList playlist;
    ConnectionHolderPointer holder ;
    
    FrameValue current_frame ;
    FrameTimer songTimer ;
    
    auto songFrameUpdate(FrameValue value) -> void ;
    auto songStopCallback(FrameValue value) -> void ;
    
    std::string currentLight ;
    std::string currentMusic ;
    int endFrame ;
    int syncInterval ;
    bool isLoaded ;
    
    mutable std::mutex access ;
    auto resetEntry() -> void ;
    auto playEntry() ->ListEntry ;

public:
    bool isPlaying ;
    bool shouldStop ;
    bool showPlaying ;
    
    Show(ConnectionHolderPointer connection_holder) ;
    ~Show() ;
    
    auto setPlayList( const PlayList &list ) ->void ;
    auto start() -> void ;

    auto loadNext() -> bool ;
    auto play() -> bool ;

    auto current() -> CurrentEntry ;
    auto currentFrame() const -> FrameValue ;
    auto setSyncInterval(int sync_interval) -> void ;
    
    
};
#endif /* Show_hpp */
