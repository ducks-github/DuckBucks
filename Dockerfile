FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-9 \
    g++-9 \
    libc6-dev \
    libc6-dev-amd64-cross \
    cmake \
    git \
    libssl-dev \
    libboost-all-dev \
    libdb5.3-dev \
    libdb++-dev \
    libevent-dev \
    && rm -rf /var/lib/apt/lists/*

# Verify db_cxx.h is installed
RUN if [ ! -f /usr/include/db_cxx.h ]; then echo "Error: db_cxx.h not found in /usr/include/"; exit 1; fi

WORKDIR /app
COPY . .
RUN chmod +x src/leveldb/build_detect_platform \
    && cd src/leveldb && ./build_detect_platform build_config.mk ./ \
    && make libleveldb.a \
    && cd .. && make -f makefile.unix
