#pragma once
#include <cstdint>
#include <functional>

struct Message {
    std::string getType() const { return "test"; }
    size_t getSize() const { return 0; }
};

class Network {
public:
    explicit Network(uint16_t port);
    bool isListening() const;
    size_t getPeerCount() const;
    void processMessages();
    void setMessageCallback(std::function<void(const Message&)> callback);
private:
    uint16_t port_;
    bool listening_{false};
};