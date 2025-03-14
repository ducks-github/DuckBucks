#include "blockchain.h"
#include <spdlog/spdlog.h>

uint64_t Blockchain::getHeight() const {
    return height_;
}

void Blockchain::processPendingTransactions() {
    // Placeholder for transaction processing
    spdlog::debug("Processing pending transactions");
}