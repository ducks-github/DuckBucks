#include "debug.h"
#include "wallet.h"
#include "blockchain.h"
#include "network.h"
#include <spdlog/spdlog.h>

int main() {
    try {
        Debug::initialize();
        Debug::enableMemoryTracing();
        
        spdlog::info("Starting Duckbucks in debug mode");
        
        // Initialize core components
        Wallet wallet("debug_wallet");
        Blockchain chain;
        Network network(6335);
        
        // Test blockchain state
        spdlog::debug("Current blockchain height: {}", chain.getHeight());
        spdlog::debug("Connected peers: {}", network.getPeerCount());
        spdlog::debug("Wallet balance: {}", wallet.getBalance().value_or(0.0));
        
        // Monitor network messages
        network.setMessageCallback([](const Message& msg) {
            spdlog::debug("Network message received: type={}, size={}", 
                         msg.getType(), msg.getSize());
        });
        
        // Start main loop
        while (true) {
            chain.processPendingTransactions();
            network.processMessages();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }
}