cmake_minimum_required(VERSION 3.16)
project(Duckbucks VERSION 0.9.0 LANGUAGES CXX)

# Use modern C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Security flags
add_compile_options(
    -Wall -Wextra -Werror
    -D_FORTIFY_SOURCE=2
    -fstack-protector-strong
    -fPIE
)

# Dependencies
find_package(OpenSSL 1.1.1 REQUIRED)
find_package(Boost 1.74 REQUIRED COMPONENTS
    system
    filesystem
    program_options
    thread
    chrono
)
find_package(RocksDB REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# Main executable
add_executable(duckbucksd
    src/main.cpp
    # ... other source files
)

target_link_libraries(duckbucksd PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    Boost::system
    Boost::filesystem
    Boost::program_options
    Boost::thread
    Boost::chrono
    RocksDB::rocksdb
)

# Add after other find_package calls
find_package(Boost REQUIRED COMPONENTS system)

# Update target_link_libraries for duckbucks_lib
target_link_libraries(duckbucks_lib PRIVATE
    spdlog::spdlog
    Boost::system
    pthread
)
