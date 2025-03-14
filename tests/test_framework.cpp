#include <catch2/catch_all.hpp>
#include "../src/wallet.h"
#include "../src/blockchain.h"
#include "../src/network.h"

TEST_CASE("Basic blockchain operations", "[blockchain]") {
    Blockchain chain;
    
    SECTION("Block creation") {
        Block block = chain.createNewBlock();
        REQUIRE(block.isValid());
        REQUIRE(block.getHeight() == chain.getHeight() + 1);
    }
    
    SECTION("Transaction validation") {
        Transaction tx;
        REQUIRE(chain.validateTransaction(tx) == true);
    }
}

TEST_CASE("Network operations", "[network]") {
    Network net(6335);  // Default Duckbucks port
    
    SECTION("Peer connection") {
        REQUIRE(net.isListening());
        REQUIRE(net.getPeerCount() == 0);
    }
}