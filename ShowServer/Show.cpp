// Copyright © 2024 Charles Kerr. All rights reserved.

#include "Show.hpp"



// =====================================================================================
// Show
// =====================================================================================

// ==================================================================================================
auto Show::runIO() -> void {
    io_context.run() ;
}

// ==================================================================================================
auto Show::tick(const asio::error_code &ec,asio::steady_timer* timer ) -> void{
    if (ec) {
        // we had an error, we just ignore? or do we send a stop playing?
        if (isPlaying){
            if (songStoppedCallback != nullptr) {
                songStoppedCallback() ;
            }
            isPlaying = false ;
            currentFrame = 0 ;
            stopAt = 0 ;
        }
        return ;
    }
    // reschedule another
     currentFrame += 1 ;
    if (currentFrame >= stopAt) {
        if (songStoppedCallback != nullptr) {
            songStoppedCallback();
        }
        isPlaying = false ;
        currentFrame = 0 ;
        stopAt = 0 ;
        return ; // We need to return  here, so we dont requeue ourselves
    }
    else if (songSyncCallback != nullptr  && ( ( currentFrame % frameSync == 0 )  || ( settleFrame == currentFrame ) )  )  {
        songSyncCallback(FrameValue(currentFrame)) ;
    }
    auto time = timer->expiry() ;
    timer->expires_at(time + std::chrono::milliseconds(framePeriod)) ;
    timer->async_wait(std::bind(&Show::tick,this,std::placeholders::_1,timer) );

}

// ====================================================================================
Show::Show():frameTimer(io_context), isPlaying(false),shouldEndShow(false), inShow(false),currentPlaylistEntry(0), currentChapter(ShowChapter::prologue),  songStoppedCallback(nullptr),  songSyncCallback(nullptr), stopImmediately(false), frameSync(270),settleFrame((10)){
    
    io_thread = std::thread(&Show::runIO,this) ;
}
// ====================================================================================
Show::~Show() {
    
    try {
        frameTimer.cancel() ;
    }
    catch(...){}
    
    if (!io_context.stopped()) {
        io_context.stop();
    }
    if (io_thread.joinable()) {
        io_thread.join();
    }
}

// ====================================================================================
auto Show::load(const std::filesystem::path &filepath) -> bool {
    return playlist.load(filepath) ;
}

// ====================================================================================
auto Show::size() const  -> size_t {
    return playlist.size() ;
}

// ====================================================================================
auto Show::setFrameSync(int sync) -> void {
    frameSync = sync ;
}

// ====================================================================================
auto Show::setSyncCallback(SongSync function) -> void {
    songSyncCallback = function ;
}
// ====================================================================================
auto Show::setSongStopped(SongStopped function) -> void {
    songStoppedCallback = function ;
}


// =====================================================================================
auto Show::start(const ListEntry &entry) -> bool {
    if ( playlist.size() < 1) {
        return false ;
    }
    inShow = true ;
    currentEntry = entry ;
    isPlaying = true ;
    currentFrame = 0 ;
    stopAt = entry.framecount ;
    frameTimer.expires_at(std::chrono::steady_clock::now() + std::chrono::milliseconds(framePeriod));
    frameTimer.async_wait(std::bind(&Show::tick,this,std::placeholders::_1,&frameTimer) );
    return true ;
}

// =====================================================================================
auto Show::getCurrentFrame() const  -> FrameValue {
    if (!isPlaying) {
        return FrameValue(0,false) ;
    }
    return FrameValue(currentFrame) ;
}

// ======================================================================================
auto Show::nextEntry() -> ListEntry {
    auto entry = ListEntry() ;
    if (shouldEndShow && currentChapter != ShowChapter::epilogue){
        currentChapter = ShowChapter::epilogue ;
        currentPlaylistEntry = 0 ;
    }
    if (currentChapter ==  ShowChapter::prologue) {
        if (currentPlaylistEntry < playlist.prologue.size()){
            entry = playlist.prologue.at(currentPlaylistEntry) ;
            currentPlaylistEntry += 1 ;
        }
        else {
            currentChapter = ShowChapter::body;
            currentPlaylistEntry = 0 ;
        }
    }
    if (currentChapter == ShowChapter::body) {
        if (playlist.body.empty() || shouldEndShow){
            currentChapter = ShowChapter::epilogue ;
            currentPlaylistEntry = 0 ;
        }
        else if (currentPlaylistEntry >= playlist.body.size() ){
            currentPlaylistEntry = 0 ;
        }
        if (!playlist.body.empty() && !shouldEndShow){
            entry = playlist.body.at(currentPlaylistEntry) ;
            currentPlaylistEntry += 1 ;
        }
    }
    if (currentChapter == ShowChapter::epilogue){
        if (currentPlaylistEntry < playlist.epilogue.size()){
            entry = playlist.epilogue.at(currentPlaylistEntry);
            currentPlaylistEntry += 1 ;
        }
    }
    
    return entry ;
}


// ======================================================================================
auto Show::stop() -> void {
    try{
        frameTimer.cancel();
    }
    catch(...){}
}
// ======================================================================================
auto Show::reset() -> void {
    
    isPlaying = false ;
    shouldEndShow = false ;
    stopImmediately = false ;
    stopAt = 0 ;
    currentFrame = 0 ;
    currentEntry= ListEntry() ;
    currentPlaylistEntry = 0 ;
    currentChapter = ShowChapter::prologue ;
    inShow = false ;
}
