#pragma once
#include <boost/asio.hpp>
#include <unordered_map>
#include <string>
#include "peer.h"
#include "message.h"

class Network {
public:
    explicit Network(uint16_t port);
    ~Network();
    
    void start();
    void stop();
    void broadcast(const Message& msg);
    void connectToPeer(const std::string& host, uint16_t port);
    
    bool isListening() const { return acceptor_ && acceptor_->is_open(); }
    size_t getPeerCount() const { return peers_.size(); }
    
    void setMessageCallback(std::function<void(const Message&)> callback) {
        messageCallback_ = std::move(callback);
    }

private:
    void startAccept();
    void handlePeer(std::shared_ptr<Peer> peer);
    void handleMessage(const Message& msg, const std::shared_ptr<Peer>& peer);
    
    boost::asio::io_context io_context_;
    std::unique_ptr<boost::asio::io_context::work> work_;
    std::unique_ptr<tcp::acceptor> acceptor_;
    std::thread io_thread_;
    
    std::unordered_map<std::string, std::shared_ptr<Peer>> peers_;
    std::function<void(const Message&)> messageCallback_;
    
    uint16_t port_;
    bool running_{false};
};