#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <queue>
#include "message.h"

using boost::asio::ip::tcp;

class Peer : public std::enable_shared_from_this<Peer> {
public:
    explicit Peer(boost::asio::io_context& io_context);
    
    void connect(const std::string& host, uint16_t port);
    void start();
    void send(const Message& msg);
    
    bool isConnected() const { return connected_; }
    std::string getAddress() const;
    
    std::function<void(const Message&)> onMessage;
    std::function<void(const std::string&)> onError;

private:
    void readHeader();
    void readBody();
    void writeMessage();
    
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    bool connected_{false};
    
    std::vector<uint8_t> read_buffer_;
    std::queue<std::vector<uint8_t>> write_queue_;
    
    static constexpr size_t HEADER_SIZE = 5;
};