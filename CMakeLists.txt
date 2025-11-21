cmake_minimum_required(VERSION 3.25)
project(ShowServer VERSION 1.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(ShowServer
    ${PROJECT_SOURCE_DIR}/ShowServer/main.cpp
    
    ${PROJECT_SOURCE_DIR}/ShowServer/ClientHolder.cpp
    ${PROJECT_SOURCE_DIR}/ShowServer/ClientHolder.hpp
    ${PROJECT_SOURCE_DIR}/ShowServer/Listener.cpp
    ${PROJECT_SOURCE_DIR}/ShowServer/Listener.hpp
    ${PROJECT_SOURCE_DIR}/ShowServer/Logger.cpp
    ${PROJECT_SOURCE_DIR}/ShowServer/Logger.hpp
    ${PROJECT_SOURCE_DIR}/ShowServer/PlayList.cpp
    ${PROJECT_SOURCE_DIR}/ShowServer/PlayList.hpp
    ${PROJECT_SOURCE_DIR}/ShowServer/ServerClient.cpp
    ${PROJECT_SOURCE_DIR}/ShowServer/ServerClient.hpp
    ${PROJECT_SOURCE_DIR}/ShowServer/ServerConfiguration.cpp
    ${PROJECT_SOURCE_DIR}/ShowServer/ServerConfiguration.hpp
    ${PROJECT_SOURCE_DIR}/ShowServer/Show.cpp
    ${PROJECT_SOURCE_DIR}/ShowServer/Show.hpp
    ${PROJECT_SOURCE_DIR}/ShowServer/StateHolder.cpp
    ${PROJECT_SOURCE_DIR}/ShowServer/StateHolder.hpp

    ${PROJECT_SOURCE_DIR}/common/network/Connection.cpp
    ${PROJECT_SOURCE_DIR}/common/network/Connection.hpp
    ${PROJECT_SOURCE_DIR}/common/network/FrameValue.cpp
    ${PROJECT_SOURCE_DIR}/common/network/FrameValue.hpp

    ${PROJECT_SOURCE_DIR}/common/packets/allpackets.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/BufferPacket.cpp
    ${PROJECT_SOURCE_DIR}/common/packets/BufferPacket.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/ErrorPacket.cpp
    ${PROJECT_SOURCE_DIR}/common/packets/ErrorPacket.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/IdentPacket.cpp
    ${PROJECT_SOURCE_DIR}/common/packets/IdentPacket.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/LoadPacket.cpp
    ${PROJECT_SOURCE_DIR}/common/packets/LoadPacket.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/NopPacket.cpp
    ${PROJECT_SOURCE_DIR}/common/packets/NopPacket.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/Packet.cpp
    ${PROJECT_SOURCE_DIR}/common/packets/Packet.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/PacketType.cpp
    ${PROJECT_SOURCE_DIR}/common/packets/PacketType.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/PlayPacket.cpp
    ${PROJECT_SOURCE_DIR}/common/packets/PlayPacket.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/ShowPacket.cpp
    ${PROJECT_SOURCE_DIR}//common/packets/ShowPacket.hpp
    ${PROJECT_SOURCE_DIR}/common/packets/SyncPacket.cpp
    ${PROJECT_SOURCE_DIR}/common/packets/SyncPacket.hpp
    
    ${PROJECT_SOURCE_DIR}/common/utility/BaseConfiguration.cpp
    ${PROJECT_SOURCE_DIR}/common/utility/BaseConfiguration.hpp
    ${PROJECT_SOURCE_DIR}/common/utility/buffer.cpp
    ${PROJECT_SOURCE_DIR}/common/utility/buffer.hpp
    ${PROJECT_SOURCE_DIR}/common/utility/dbgutil.hpp
    ${PROJECT_SOURCE_DIR}/common/utility/mapfile.cpp
    ${PROJECT_SOURCE_DIR}/common/utility/mapfile.hpp
    ${PROJECT_SOURCE_DIR}/common/utility/strutil.hpp
    ${PROJECT_SOURCE_DIR}/common/utility/timeutil.cpp
    ${PROJECT_SOURCE_DIR}/common/utility/timeutil.hpp
)

target_compile_definitions(ShowServer PRIVATE
        ASIO_STANDALONE
)
if (WIN32)
    target_compile_definitions(ShowServer PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
        $<$<CONFIG:Debug>:_DEBUG=1>
        _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
        _WIN32_WINNT=0x0A00
    )

    target_compile_options(ShowServer PRIVATE
        $<$<CONFIG:Release>:/O2>
    )

    target_link_libraries(ShowServer
        PUBLIC
            ole32
    )
else()
    target_compile_definitions(ShowServer PRIVATE
        $<$<CONFIG:Debug>:DEBUG=1>
    )

    target_compile_options(ShowServer PRIVATE
        $<$<CONFIG:Release>:-O2>
        -Wno-deprecated-declarations
    )
    target_link_libraries(ShowServer
        PUBLIC
            pthread
    )
    
endif (WIN32)

target_include_directories(ShowServer
    PUBLIC
        ${PROJECT_SOURCE_DIR}/ShowServer/
        ${PROJECT_SOURCE_DIR}/common/
        ${PROJECT_SOURCE_DIR}/thirdparty/asio-1.28/
)
