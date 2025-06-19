cmake_minimum_required(VERSION 3.10)
project(Client VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FREERDP3 REQUIRED freerdp3)


add_executable(Client client.cxx)
target_include_directories(Client PRIVATE ${FREERDP3_INCLUDE_DIRS})
target_link_libraries(Client PRIVATE ${FREERDP3_LIBRARIES})

