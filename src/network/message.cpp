#include "message.h"
#include <stdexcept>

Message::Message(MessageType type, std::vector<uint8_t> payload)
    : type_(type), payload_(std::move(payload)) {}

std::vector<uint8_t> Message::serialize() const {
    std::vector<uint8_t> result;
    result.reserve(payload_.size() + 5);
    
    // Message format: [4 bytes length][1 byte type][payload]
    uint32_t length = static_cast<uint32_t>(payload_.size());
    result.push_back((length >> 24) & 0xFF);
    result.push_back((length >> 16) & 0xFF);
    result.push_back((length >> 8) & 0xFF);
    result.push_back(length & 0xFF);
    
    result.push_back(static_cast<uint8_t>(type_));
    result.insert(result.end(), payload_.begin(), payload_.end());
    
    return result;
}

Message Message::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 5) {
        throw std::runtime_error("Message too short");
    }
    
    uint32_t length = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    MessageType type = static_cast<MessageType>(data[4]);
    
    if (data.size() != length + 5) {
        throw std::runtime_error("Invalid message length");
    }
    
    std::vector<uint8_t> payload(data.begin() + 5, data.end());
    return Message(type, std::move(payload));
}