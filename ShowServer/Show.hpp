// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef Show_hpp
#define Show_hpp

#include <string>
#include <thread>
#include <filesystem>

#include "asio.hpp"

#include "network/FrameValue.hpp"
#include "PlayList.hpp"


// =====================================================================================
// Show
// =====================================================================================

using SongStopped = std::function<void()> ;
using SongSync = std::function<void(const FrameValue &)> ;
class Show {
    static constexpr int framePeriod = 37 ; // 37 milliseconds
    enum ShowChapter {
        prologue,body,epilogue
    };
    
    // Timer related items

    asio::io_context io_context ;
    asio::executor_work_guard<asio::io_context::executor_type> ioguard{asio::make_work_guard(io_context)} ;
    asio::steady_timer frameTimer ;
    
    auto runIO() -> void ;
    std::thread io_thread ;
    
    auto tick(const asio::error_code &ec,asio::steady_timer* timer ) -> void;

    
    PlayList playlist ;

    ShowChapter currentChapter ;
    int currentPlaylistEntry ;
    
    
    SongStopped songStoppedCallback ;
    SongSync songSyncCallback ;
    
    int frameSync ;
    int stopAt ;
    int currentFrame ;
    int settleFrame ;

public:
    bool isPlaying ;
    bool shouldEndShow ;
    bool stopImmediately;
    ListEntry currentEntry ;
    bool inShow ;
    Show() ;
    ~Show() ;
    auto load(const std::filesystem::path &filepath) -> bool ;
    auto size() const  -> size_t ;
    auto setFrameSync(int sync) -> void ;
    auto setSyncCallback(SongSync function) -> void ;
    auto setSongStopped(SongStopped function) -> void ;

    auto getCurrentFrame() const  -> FrameValue ;
    auto nextEntry() -> ListEntry ;
    auto start(const ListEntry &entry) -> bool ;
    auto stop() -> void ;
    auto reset() -> void ;
};

#endif /* Show_hpp */
