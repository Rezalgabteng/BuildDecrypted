cmake_minimum_required(VERSION 3.10)
project(BuildDecrypted)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Base flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -Wno-error=format")

# Ignore undefined reference errors from Android library
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--undefined=__errno -Wl,--allow-shlib-undefined")

# Source files
set(SOURCES
    main.cpp
    Kernel.cpp
)

set(HEADERS
    Kernel.hpp
    DecryptEngine.hpp
    paradise_api.h
)

# Add executable
add_executable(decrypt_engine ${SOURCES} ${HEADERS})

# Link against paradise library
target_link_libraries(decrypt_engine PRIVATE pthread)

# Try to link against libparadise_api.a if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libparadise_api.a")
    target_link_libraries(decrypt_engine PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/libparadise_api.a)
    
    message(STATUS "Linking against local libparadise_api.a")
    message(STATUS "Note: libparadise_api.a compiled for Android")
    message(STATUS "Warning: Ignoring undefined __errno symbol")
else()
    message(WARNING "libparadise_api.a not found - linking may fail")
    message(WARNING "Ensure libparadise_api.a is in project root")
endif()

# Include directories
target_include_directories(decrypt_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Installation
install(TARGETS decrypt_engine DESTINATION bin)

# Print build info
message(STATUS "")
message(STATUS "╔══════════════════════════════════════╗")
message(STATUS "║     BuildDecrypted Build Config     ║")
message(STATUS "╠══════════════════════════════════════╣")
message(STATUS "║ Target: com.proximabeta.mf.uamo     ║")
message(STATUS "║ Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "║ CXX Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "║ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "╚══════════════════════════════════════╝")
message(STATUS "")
