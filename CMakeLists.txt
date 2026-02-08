cmake_minimum_required(VERSION 3.30)
project(MultiplayerSample)

include(FetchContent)

set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
        raylib
        GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
)

FetchContent_Declare(
        lua
        GIT_REPOSITORY "https://gitlab.com/codelibre/lua/lua-cmake.git"
        GIT_TAG "lua-cmake-5.4"
)

set(raylib_USE_STATIC_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(lua)

set(SOURCES
        src/main.cpp
        src/network/client.cpp
        src/network/server.cpp
        src/network/packets.cpp
        src/util/net.cpp
        src/util/dev/console/console.cpp
        src/util/draw.cpp
        src/util/numbers.cpp
        src/util/dev/console/command/registry.cpp
        src/util/dev/console/command/commands/core_command.cpp
        src/util/dev/console/command/auto_completion.cpp
        src/manager/ConsoleManager.cpp
        src/manager/ClientManager.cpp
        src/manager/ServerManager.cpp
        src/input/input.cpp
        src/input/keybind.cpp
        src/player.cpp
)
set(HEADERS
        include/network/client.h
        include/network/packets.h
        include/util/net.h
        include/network/server.h
        include/util/draw.h
        include/util/dev/console/console.h
        include/util/numbers.h
        include/util/dev/console/command/registry.h
        include/util/dev/console/command/commands/core_command.h
        include/util/dev/console/command/auto_completion.h
        include/manager/ServerManager.h
        include/manager/ClientManager.h
        include/manager/ConsoleManager.h
        include/input/input.h
        include/input/keybind.h
        include/player.h
)

set(CMAKE_CXX_STANDARD 20)

add_executable(MultiplayerSample ${SOURCES} ${HEADERS})

target_include_directories(MultiplayerSample
        PUBLIC
        include
)

set_property(TARGET MultiplayerSample PROPERTY C_STANDARD 17)

target_link_libraries(MultiplayerSample PRIVATE raylib)
target_link_libraries(MultiplayerSample PRIVATE lua_library)
target_link_libraries(MultiplayerSample PRIVATE ws2_32)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(MultiplayerSample PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/")
endif ()
target_compile_definitions(MultiplayerSample PUBLIC GAME_VERSION="1.0.0")
target_compile_definitions(MultiplayerSample PUBLIC MEMORY_RUNTIME_SAFETY=1)

if(WIN32)
    target_compile_definitions(MultiplayerSample PUBLIC PLATFORM_WINDOWS)

    target_link_libraries(MultiplayerSample PRIVATE ws2_32)
elseif(APPLE)
    target_compile_definitions(MultiplayerSample PUBLIC PLATFORM_MACOS)
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(MultiplayerSample PUBLIC PLATFORM_LINUX)
endif()