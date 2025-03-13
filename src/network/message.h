#pragma once
#include <string>
#include <vector>
#include <cstdint>

enum class MessageType : uint8_t {
    HANDSHAKE = 0,
    PING = 1,
    PONG = 2,
    GET_PEERS = 3,
    PEERS = 4,
    GET_BLOCKS = 5,
    BLOCKS = 6,
    TRANSACTION = 7
};

class Message {
public:
    Message(MessageType type, std::vector<uint8_t> payload);
    
    // Serialization
    std::vector<uint8_t> serialize() const;
    static Message deserialize(const std::vector<uint8_t>& data);
    
    MessageType getType() const { return type_; }
    const std::vector<uint8_t>& getPayload() const { return payload_; }
    size_t getSize() const { return payload_.size(); }

private:
    MessageType type_;
    std::vector<uint8_t> payload_;
};