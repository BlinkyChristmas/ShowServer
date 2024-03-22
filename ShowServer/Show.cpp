// Copyright © 2024 Charles Kerr. All rights reserved.

#include "Show.hpp"

#include "utility/dbgutil.hpp"

using namespace std::string_literals ;

// =====================================================================================
auto CurrentEntry::isValid()  -> bool {
    return framecount != 0 ;
}

// =====================================================================================
auto Show::songFrameUpdate(FrameValue value) -> void {
    current_frame = value ;
    //DBGMSG(std::cout, "Sending sync: "s + std::to_string(value.value())) ;
    holder->sendSync(current_frame);
}

// =====================================================================================
auto Show::songStopCallback(FrameValue value)->void {
    current_frame = value ;
    isPlaying = false ;
    DBGMSG(std::cout, "Sending stop: "s + std::to_string(value.value())) ;
    holder->sendPlay(false) ;
    resetEntry() ;
}
// =====================================================================================
auto Show::playEntry() ->ListEntry {
    auto entry = ListEntry();
    if ( state == ShowState::prologue) {
        if (playlist.prologue.size() > nextEntry) {
            entry = playlist.prologue.at(nextEntry) ;
            nextEntry++ ;
        }
        else {
            state = ShowState::body ;
            nextEntry = 0 ;
        }
    }
    if ( state == ShowState::body) {
        if (shouldStop || playlist.body.size() == 0){
            state = ShowState::epilogue;
            nextEntry = 0 ;
        }
        else {
            if (playlist.body.size() <= nextEntry) {
                nextEntry = 0 ;
            }
            entry = playlist.body.at(nextEntry) ;
            nextEntry++ ;
        }
    }
    if (state == ShowState::epilogue){
        if (playlist.epilogue.size() <= nextEntry) {
            isPlaying = false ;
            
        }
        else {
            entry = playlist.epilogue.at(nextEntry) ;
            nextEntry++ ;
        }

    }
    return entry ;
}

// =====================================================================================
Show::Show(ConnectionHolderPointer connection_holder) {
    holder = connection_holder ;
    setPlayList(PlayList());
    showPlaying = false ;
    songTimer.settleFrame = 10 ;
    auto setUpdateCallback(FrameTimerUpdateCallback routine, int frequency = 1) -> void ;
    auto setStopCallback(FrameTimerUpdateCallback routine) -> void ;

    songTimer.setUpdateCallback(std::bind(&Show::songFrameUpdate,this,std::placeholders::_1)) ;
    songTimer.setStopCallback(std::bind(&Show::songStopCallback,this,std::placeholders::_1));

    syncInterval = 270 ;
}

//=======================================================================================
Show::~Show() {
    if (isPlaying) {
        songTimer.stop() ;
        holder->sendPlay(false);
    }
}
// ===============================================================================================
auto Show::setPlayList(const PlayList &list) -> void {
    if (!showPlaying) {
        playlist = list ;
        state = ShowState::prologue ;
        isPlaying = false ;
        endFrame = 0 ;
        nextEntry = 0 ;
        shouldStop = false ;
        isLoaded = false ;
        currentMusic = ""s ;
        currentLight = ""s ;
    }
}

// ===============================================================================================
auto Show::current() -> CurrentEntry {
    auto lock = std::lock_guard(access) ;
    auto entry = CurrentEntry() ;
    if (isLoaded) {
        if (endFrame - current_frame.value()  > 810) { // Have to be outside 30 seconds from the end
            entry.framecount = this->endFrame ;
            entry.music = this->currentMusic ;
            entry.light = this->currentLight ;
        }
    }
    return entry ;
}
// ===============================================================================================
auto Show::currentFrame() const -> FrameValue {
    return current_frame ;
}

// ===============================================================================================
auto Show::start() -> void {
    showPlaying = true ;
    shouldStop = false ;
    isLoaded = false ;
    isPlaying = false ;
    state = ShowState::prologue ;
    nextEntry = 0 ;
}

// ===============================================================================================
auto Show::resetEntry() -> void {
    isLoaded = false ;
    currentLight = ""s;
    currentMusic = ""s ;
    endFrame = 0 ;
}

// ===============================================================================================
auto Show::loadNext() -> bool {
    resetEntry() ;
    auto entry = playEntry() ;
    if (!entry.valid()) {
        return false ;
    }
    else {
        auto lock = std::lock_guard(access) ;
        songTimer.stop() ;
        currentLight = entry.lightName ;
        currentMusic = entry.musicName ;
        endFrame = entry.framecount ;
        isLoaded = true ;
        DBGMSG(std::cout,"Loading "s + currentMusic + " , "s + currentLight);
        holder->sendLoad(currentMusic, currentLight) ;
        return true ;
    }
}
// ==============================================================================================
auto Show::play() -> bool {
    if (!isLoaded || isPlaying) {
        return false ;
    }
    isPlaying = true ;
    current_frame = FrameValue() ;
    DBGMSG(std::cout,"Starting play");
    songTimer.start(syncInterval,endFrame) ;
    holder->sendPlay(true,current_frame) ;
    return isPlaying ;
}

// ==============================================================================================
auto Show::setSyncInterval(int sync_interval) -> void {
    syncInterval = sync_interval ;
}
