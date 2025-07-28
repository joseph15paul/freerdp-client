cmake_minimum_required(VERSION 3.10)
project(Client VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Paths to your local FreeRDP source and build
set(FREERDP_SRC_DIR "${CMAKE_SOURCE_DIR}/../freerdp/FreeRDP")
set(FREERDP_BUILD_DIR "${FREERDP_SRC_DIR}/build")

# Add Boost (required by WebSocket++)
find_package(Boost REQUIRED COMPONENTS system thread)
include_directories(${CMAKE_SOURCE_DIR}/include)
# Define the executable
add_executable(Client client.cxx)

# Include FreeRDP and WinPR headers (both source and generated)
target_include_directories(Client PRIVATE
    "${FREERDP_SRC_DIR}/include"
    "${FREERDP_SRC_DIR}/winpr/include" 
    
    "${FREERDP_BUILD_DIR}/include"
    "${FREERDP_BUILD_DIR}/winpr/include"

    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/websocketpp
)

# Link against the locally built .so files (assumes you didn't install system-wide)
target_link_directories(Client PRIVATE
    "${FREERDP_BUILD_DIR}/libfreerdp"
    "${FREERDP_BUILD_DIR}/winpr/libwinpr/"
)

# Required WinPR components
target_link_libraries(Client PRIVATE
    freerdp3
    freerdp-client3
    winpr3  

    ${Boost_LIBRARIES}
    pthread
)

