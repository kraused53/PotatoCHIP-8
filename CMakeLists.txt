cmake_minimum_required( VERSION 3.20 )
project( PotatoCHIP LANGUAGES CXX )

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )

include( FetchContent )

# Include SDL2
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.30.2  # Use a specific release tag or commit hash
)

# Make the content available
FetchContent_MakeAvailable(SDL2)

# Bring in my logger library
FetchContent_Declare(
    clogg
    GIT_REPOSITORY https://github.com/kraused53/CLogg.git
    GIT_TAG V1.1
)

FetchContent_MakeAvailable( clogg )

# CHIP 8 Emulator
add_library(
  CHIP8
  src/core/emulator.cpp
  src/core/CHIP.cpp
)

target_include_directories(
  CHIP8
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
  CHIP8
  PUBLIC
    SDL2::SDL2
    SDL2::SDL2main
  PRIVATE
    clogg::clogg
)

# Executable
add_executable(
  potato_chip
  src/main.cpp
)

target_link_libraries(
  potato_chip
  PRIVATE
    CHIP8
)