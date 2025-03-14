FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-12 \
    g++-12 \
    libc6-dev \
    libc6-dev-amd64-cross \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    libdb5.3-dev \
    libdb++-dev \
    libevent-dev \
    libsnappy-dev \
    liblz4-dev \
    libzstd-dev \
    && rm -rf /var/lib/apt/lists/*

# Verify db_cxx.h is installed
RUN if [ ! -f /usr/include/db_cxx.h ]; then echo "Error: db_cxx.h not found in /usr/include/"; exit 1; fi

WORKDIR /app
COPY . .

# Build using CMake
RUN mkdir build && cd build \
    && cmake .. \
    && make -j$(nproc)
