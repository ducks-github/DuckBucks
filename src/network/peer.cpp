#include "peer.h"
#include <spdlog/spdlog.h>

Peer::Peer(boost::asio::io_context& io_context)
    : io_context_(io_context), socket_(io_context) {}

void Peer::connect(const std::string& host, uint16_t port) {
    tcp::resolver resolver(io_context_);
    auto endpoints = resolver.resolve(host, std::to_string(port));
    
    boost::asio::async_connect(socket_, endpoints,
        [this](const boost::system::error_code& ec, const tcp::endpoint&) {
            if (!ec) {
                connected_ = true;
                spdlog::debug("Connected to peer: {}", getAddress());
                start();
            } else {
                if (onError) {
                    onError(ec.message());
                }
            }
        });
}

void Peer::start() {
    readHeader();
}

void Peer::send(const Message& msg) {
    bool write_in_progress = !write_queue_.empty();
    write_queue_.push(msg.serialize());
    
    if (!write_in_progress) {
        writeMessage();
    }
}

std::string Peer::getAddress() const {
    try {
        return socket_.remote_endpoint().address().to_string() + ":" +
               std::to_string(socket_.remote_endpoint().port());
    } catch (...) {
        return "unknown";
    }
}

void Peer::readHeader() {
    read_buffer_.resize(HEADER_SIZE);
    
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_buffer_, HEADER_SIZE),
        [this](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                uint32_t body_size = (read_buffer_[0] << 24) |
                                   (read_buffer_[1] << 16) |
                                   (read_buffer_[2] << 8) |
                                   read_buffer_[3];
                read_buffer_.resize(HEADER_SIZE + body_size);
                readBody();
            } else {
                connected_ = false;
                if (onError) {
                    onError(ec.message());
                }
            }
        });
}

void Peer::readBody() {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_buffer_.data() + HEADER_SIZE,
                           read_buffer_.size() - HEADER_SIZE),
        [this](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                try {
                    auto msg = Message::deserialize(read_buffer_);
                    if (onMessage) {
                        onMessage(msg);
                    }
                } catch (const std::exception& e) {
                    spdlog::error("Failed to parse message: {}", e.what());
                }
                readHeader();
            } else {
                connected_ = false;
                if (onError) {
                    onError(ec.message());
                }
            }
        });
}

void Peer::writeMessage() {
    boost::asio::async_write(socket_,
        boost::asio::buffer(write_queue_.front()),
        [this](const boost::system::error_code& ec, std::size_t) {
            if (!ec) {
                write_queue_.pop();
                if (!write_queue_.empty()) {
                    writeMessage();
                }
            } else {
                connected_ = false;
                if (onError) {
                    onError(ec.message());
                }
            }
        });
}