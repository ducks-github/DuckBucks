#pragma once

#include <spdlog/spdlog.h>
#include <string_view>

class Debug {
public:
    static void initialize() {
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
        spdlog::set_level(spdlog::level::debug);
    }

    static void enableMemoryTracing() {
        // Track memory allocations
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    }
};