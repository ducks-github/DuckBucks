#include "network.h"
#include <spdlog/spdlog.h>

Network::Network(uint16_t port) : port_(port) {
    work_ = std::make_unique<boost::asio::io_context::work>(io_context_);
}

Network::~Network() {
    stop();
}

void Network::start() {
    if (running_) return;
    
    try {
        // Start acceptor
        tcp::endpoint endpoint(tcp::v4(), port_);
        acceptor_ = std::make_unique<tcp::acceptor>(io_context_, endpoint);
        
        startAccept();
        
        // Start IO thread
        running_ = true;
        io_thread_ = std::thread([this]() {
            try {
                io_context_.run();
            } catch (const std::exception& e) {
                spdlog::error("IO context error: {}", e.what());
            }
        });
        
        spdlog::info("Network started on port {}", port_);
    } catch (const std::exception& e) {
        spdlog::error("Failed to start network: {}", e.what());
        throw;
    }
}

void Network::stop() {
    if (!running_) return;
    
    running_ = false;
    
    // Stop accepting new connections
    if (acceptor_) {
        acceptor_->close();
    }
    
    // Disconnect all peers
    peers_.clear();
    
    // Stop IO context
    work_.reset();
    io_context_.stop();
    
    if (io_thread_.joinable()) {
        io_thread_.join();
    }
    
    spdlog::info("Network stopped");
}

void Network::broadcast(const Message& msg) {
    for (const auto& [_, peer] : peers_) {
        if (peer->isConnected()) {
            peer->send(msg);
        }
    }
}

void Network::connectToPeer(const std::string& host, uint16_t port) {
    auto peer = std::make_shared<Peer>(io_context_);
    handlePeer(peer);
    peer->connect(host, port);
}

void Network::startAccept() {
    auto peer = std::make_shared<Peer>(io_context_);
    
    acceptor_->async_accept(peer->socket_,
        [this, peer](const boost::system::error_code& ec) {
            if (!ec) {
                spdlog::debug("Accepted connection from: {}", 
                             peer->getAddress());
                handlePeer(peer);
                peer->start();
            }
            
            if (running_) {
                startAccept();
            }
        });
}

void Network::handlePeer(std::shared_ptr<Peer> peer) {
    peer->onMessage = [this, weak_peer = std::weak_ptr<peer>(peer)]
                     (const Message& msg) {
        if (auto peer_ptr = weak_peer.lock()) {
            handleMessage(msg, peer_ptr);
        }
    };
    
    peer->onError = [this, weak_peer = std::weak_ptr<peer>(peer)]
                    (const std::string& error) {
        if (auto peer_ptr = weak_peer.lock()) {
            spdlog::error("Peer error {}: {}", 
                         peer_ptr->getAddress(), error);
            peers_.erase(peer_ptr->getAddress());
        }
    };
    
    peers_[peer->getAddress()] = peer;
}

void Network::handleMessage(const Message& msg, 
                          const std::shared_ptr<Peer>& peer) {
    switch (msg.getType()) {
        case MessageType::HANDSHAKE:
            // Handle peer handshake
            spdlog::debug("Received handshake from {}", peer->getAddress());
            break;
            
        case MessageType::GET_PEERS:
            // Send peer list
            std::vector<uint8_t> peer_data;
            for (const auto& [addr, _] : peers_) {
                // Format: peer list data
            }
            peer->send(Message(MessageType::PEERS, peer_data));
            break;
            
        default:
            if (messageCallback_) {
                messageCallback_(msg);
            }
            break;
    }
}