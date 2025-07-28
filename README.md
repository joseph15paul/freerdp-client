## Build Instructions

To build the project using CMake for the first time, follow these steps:
```sh
#Needs websocketpp repo to be cloned in the same directory
git clone https://github.com/zaphoyd/websocketpp.git

#Needs json library
wget https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -P include/nlohmann/


# Create and navigate to the build directory
mkdir build
cd build

# Generate the build files
cmake ..

# Build the project
cmake --build .
