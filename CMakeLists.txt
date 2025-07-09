cmake_minimum_required(VERSION 3.10)
project(Client VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Paths to your local FreeRDP source and build
set(FREERDP_SRC_DIR "${CMAKE_SOURCE_DIR}/../freerdp/FreeRDP")
set(FREERDP_BUILD_DIR "${FREERDP_SRC_DIR}/build")
set(WITH_CLIENT OFF)
set(WITH_CHANNELS ON)
set(WITH_SERVER OFF)  # if you don't want Wayland
# Define the executable
# add_subdirectory("${FREERDP_BUILD_DIR}/channels")
add_executable(Client client.cxx)
#add_executable(Client minic_client.cxx)
# Include FreeRDP and WinPR headers (both source and generated)
target_include_directories(Client PRIVATE
    "${FREERDP_SRC_DIR}/include"
    "${FREERDP_SRC_DIR}/winpr/include" 
    "${FREERDP_SRC_DIR}/client"
    "${FREERDP_SRC_DIR}/channels"
    "${FREERDP_SRC_DIR}/channels/client"
    
    "${FREERDP_BUILD_DIR}/include"
    "${FREERDP_BUILD_DIR}/winpr/include"
        "${FREERDP_BUILD_DIR}/client"
        "${FREERDP_BUILD_DIR}/channels"
        "${FREERDP_BUILD_DIR}/channels/client"
)

# Link against the locally built .so files (assumes you didn't install system-wide)
target_link_directories(Client PRIVATE
    "${FREERDP_BUILD_DIR}/libfreerdp"
    "${FREERDP_BUILD_DIR}/winpr/libwinpr/"
        "${FREERDP_BUILD_DIR}/client"
        "${FREERDP_BUILD_DIR}/channels"
        "${FREERDP_BUILD_DIR}/channels/client"
)

# Required WinPR components
target_link_libraries(Client PRIVATE
 "${FREERDP_BUILD_DIR}/channels/drdynvc/client/CMakeFiles/drdynvc-client.dir/drdynvc_main.c.o"
#   "${FREERDP_BUILD_DIR}/channels/drdynvc/client/CMakeFiles/drdynvc-client.dir/drdynvc_plugin.c.o"
    freerdp3
    freerdp-client3
    winpr3  

)

