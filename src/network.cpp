#include "network.h"
#include <spdlog/spdlog.h>

Network::Network(uint16_t port) : port_(port) {
    spdlog::debug("Network initialized on port {}", port);
    listening_ = true;
}

bool Network::isListening() const {
    return listening_;
}

size_t Network::getPeerCount() const {
    return 0;
}

void Network::processMessages() {
    // Placeholder for message processing
    spdlog::debug("Processing network messages");
}

void Network::setMessageCallback(std::function<void(const Message&)> callback) {
    // Store callback for later use
}