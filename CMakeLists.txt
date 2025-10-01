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
include_directories(${CMAKE_SOURCE_DIR}/rdp)
# Define the executable
add_executable(Client client.cxx)
add_library(rdp rdp/RdpClient.cxx rdp/RdpClient.hxx)
add_library(delta_conversion rdp/delta_conversion.cxx rdp/delta_conversion.hxx)

# Include FreeRDP and WinPR headers (both source and generated)
# target_include_directories(rdp PUBLIC
#     "${FREERDP_INCLUDE_DIR}/freerdp3/"
#     "${FREERDP_INCLUDE_DIR}/winpr3" 
# )
# target_include_directories(client PUBLIC
#     "${CMAKE_SOURCE_DIR}/MobiControl/LinuxAgent/agent/src/rdp"
# )
# # # Link against the locally built .so files (assumes you didn't install system-wide)
# target_link_directories(client PUBLIC
#     "${FREERDP_LIB_DIR}/"
#     "${FREERDP_LIB_DIR}/"
# )

# target_link_libraries(client PUBLIC 
#     /src/MobiControl/LinuxAgent/vcpkg_installed/x64-linux/lib/libfreerdp3.a
#     /src/MobiControl/LinuxAgent/vcpkg_installed/x64-linux/lib/libfreerdp-client3.a
#     /src/MobiControl/LinuxAgent/vcpkg_installed/x64-linux/lib/libwinpr3.a
#     /src/MobiControl/LinuxAgent/vcpkg_installed/x64-linux/lib/freerdp3/libremdesk-common.a
#     delta_conversion
#     freerdp3
#     freerdp-client3
#     winpr3
#     json-c
# )

# Include FreeRDP and WinPR headers (both source and generated)
target_include_directories(rdp PRIVATE
    "${FREERDP_SRC_DIR}/include"
    "${FREERDP_SRC_DIR}/winpr/include" 
    
    "${FREERDP_BUILD_DIR}/include"
    "${FREERDP_BUILD_DIR}/winpr/include"

    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/websocketpp
    ${CMAKE_SOURCE_DIR}/rdp
)
target_include_directories(Client PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/websocketpp
    ${CMAKE_SOURCE_DIR}/rdp
)
# Link against the locally built .so files (assumes you didn't install system-wide)
target_link_directories(rdp PRIVATE
    "${FREERDP_BUILD_DIR}/libfreerdp"
    "${FREERDP_BUILD_DIR}/winpr/libwinpr/"
)

# Required WinPR components
target_link_libraries(rdp PRIVATE
    freerdp3
    freerdp-client3
    winpr3  
    delta_conversion
)

target_link_libraries(Client PRIVATE 
    rdp
    ${Boost_LIBRARIES}
    pthread
)

