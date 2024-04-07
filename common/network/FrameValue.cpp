// Copyright © 2024 Charles Kerr. All rights reserved.

#include "FrameValue.hpp"

#include <algorithm>

// ==================================================================================================
// This represents the "sync value" (frame number) that the client should be on.
// It time stamps itself on creation, and allows for an adjustment when the value is called
// ==================================================================================================

// ==================================================================================================
FrameValue::FrameValue(int frame_value,bool no_adjust):frame(frame_value),noadjust(no_adjust),timestamp(util::ourclock::now()) {}

// ==================================================================================================
auto FrameValue::value() const -> int {
    if (noadjust) {
        return frame;
    }
    auto adjustment = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(util::ourclock::now() - timestamp).count()/37) ;
    return frame + adjustment ;
}

// ==================================================================================================
auto FrameValue::rawValue() -> int {
    return frame ;
}

