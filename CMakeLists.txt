cmake_minimum_required(VERSION 3.10)
project(Client VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works
set(WITH_CLIENT ON)
set(WITH_CLIENT_COMMON ON)
set(WITH_SAMPLE ON)
set(WITH_WAYLAND ON)  # if you don't want Wayland
set(FREERDP_VENDOR "Custom")
find_package(PkgConfig REQUIRED)
pkg_check_modules(FREERDP3 REQUIRED freerdp3)

#add_subdirectory(client)
add_executable(Client client.cxx)
#add_executable(Client basithDemo.cxx)

target_link_libraries(Client PRIVATE winpr3)
target_include_directories(Client PRIVATE ${FREERDP3_INCLUDE_DIRS})
target_link_libraries(Client PRIVATE ${FREERDP3_LIBRARIES})
