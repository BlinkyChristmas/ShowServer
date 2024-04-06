// Copyright © 2024 Charles Kerr. All rights reserved.

#include "ClientHolder.hpp"

#include "packets/allpackets.hpp"

// ================================================================================
auto ClientHolder::add(ClientPointer client) -> void {
    auto lock = std::lock_guard(accessControl);
    clients.push_back(client) ;
}

// ================================================================================
auto ClientHolder::send(const Packet &packet) -> void {
    auto lock = std::lock_guard(accessControl) ;
    for (auto iter = clients.begin();iter!= clients.end();){
        if ((*iter)->is_open()){
            if ((*iter)->send(packet) ) {
                iter++ ;
            }
            else {
                (*iter)->close() ;
                iter = clients.erase(iter) ;
            }
        }
        else {
            iter = clients.erase(iter) ;
            
        }
    }
}

// ================================================================================
auto ClientHolder::sendSync(const FrameValue &frame) -> void {
    auto lock = std::lock_guard(accessControl) ;
    for (auto iter = clients.begin();iter!= clients.end();){
        if ((*iter)->is_open()){
            auto packet = SyncPacket(frame.value());
            if ((*iter)->send(packet) ) {
                iter++ ;
            }
            else {
                (*iter)->close() ;
                iter = clients.erase(iter) ;
            }
        }
        else {
            iter = clients.erase(iter) ;
            
        }
    }
}

// ============================================================================
auto ClientHolder::sendPlay(bool state, const FrameValue &frame) -> void {
    auto lock = std::lock_guard(accessControl) ;
    for (auto iter = clients.begin();iter!= clients.end();){
        if ((*iter)->is_open()){
            auto packet = PlayPacket(state,frame.value());
            if ((*iter)->send(packet) ) {
                iter++ ;
            }
            else {
                (*iter)->close() ;
                iter = clients.erase(iter) ;
            }
        }
        else {
            iter = clients.erase(iter) ;
            
        }
    }
}

// ============================================================================
auto ClientHolder::sendNop() -> void {
    auto packet = NopPacket() ;
    this->send(packet) ;
}

// ============================================================================
auto ClientHolder::sendLoad(const std::string &music, const std::string &light) -> void {
    auto packet = LoadPacket(music, light) ;
    this->send(packet) ;
}

// ============================================================================
auto ClientHolder::sendShow(bool state) -> void {
    auto packet = ShowPacket(state) ;
    this->send(packet) ;
}

// ============================================================================
auto ClientHolder::clear() -> void {
    auto lock = std::lock_guard(accessControl) ;
    for (auto &entry:clients) {
        if (entry->is_open()) {
            entry->close() ;
        }
    }
    clients.clear() ;
}
