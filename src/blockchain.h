#pragma once
#include <cstdint>

class Blockchain {
public:
    uint64_t getHeight() const;
    void processPendingTransactions();
private:
    uint64_t height_{0};
};