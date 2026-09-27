cmake_minimum_required(VERSION 3.16)
project(ShadowStrike LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics audio window system REQUIRED)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(shadowstrike ${GAME_SOURCES})
target_include_directories(shadowstrike PRIVATE src)
target_link_libraries(shadowstrike PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)
