cmake_minimum_required(VERSION 3.20)
project(Duckbucks VERSION 0.9.0 LANGUAGES CXX)

# Use modern C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Security flags
add_compile_options(
    -Wall -Wextra -Werror
    -D_FORTIFY_SOURCE=3
    -fstack-protector-strong
    -fPIE
    -fstack-clash-protection
    -fcf-protection=full
    -Wl,-z,relro,-z,now
)

# Dependencies
find_package(OpenSSL 3.0.0 REQUIRED)
find_package(Boost 1.82.0 REQUIRED COMPONENTS
    system
    filesystem
    program_options
    thread
    chrono
)
find_package(RocksDB 8.10.0 REQUIRED)
find_package(spdlog 1.12.0 REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/addrman.cpp
    src/alert.cpp
    src/bitcoinrpc.cpp
    src/bloom.cpp
    src/checkpoints.cpp
    src/crypter.cpp
    src/db.cpp
    src/hash.cpp
    src/init.cpp
    src/key.cpp
    src/keystore.cpp
    src/leveldb.cpp
    src/net.cpp
    src/netbase.cpp
    src/protocol.cpp
    src/rpcblockchain.cpp
    src/rpcdump.cpp
    src/rpcmining.cpp
    src/rpcnet.cpp
    src/rpcrawtransaction.cpp
    src/rpcwallet.cpp
    src/script.cpp
    src/scrypt.cpp
    src/scrypt-sse2.cpp
    src/sync.cpp
    src/txdb.cpp
    src/util.cpp
    src/version.cpp
    src/wallet.cpp
    src/walletdb.cpp
)

# Main executable
add_executable(duckbucksd ${SOURCES})

target_include_directories(duckbucksd PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIR}
    ${RocksDB_INCLUDE_DIRS}
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
    spdlog::spdlog
    pthread
    dl
    z
    bz2
    lz4
    zstd
    snappy
)

# Library target
add_library(duckbucks_lib STATIC ${SOURCES})

target_include_directories(duckbucks_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIR}
    ${RocksDB_INCLUDE_DIRS}
)

target_link_libraries(duckbucks_lib PUBLIC
    spdlog::spdlog
    Boost::system
    pthread
)
