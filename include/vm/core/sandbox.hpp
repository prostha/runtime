#pragma once

#include "../include/vm/core/allocator.hpp"

extern "C" {
#include <lua.h>
}

namespace polyblox {

    inline constexpr size_t SANDBOX_MEMORY = 64 * 1024 * 1024;
    inline constexpr int SANDBOX_INSTRUCTIONS = 1000000;
    inline constexpr int MAX_COROUTINES = 16;
    inline constexpr size_t MAX_STRING = 1024 * 1024;

    enum class Context {
        SANDBOX_SERVER,
        SANDBOX_CLIENT
    };

    class Sandbox {
    public:
        explicit Sandbox(Context context);
        ~Sandbox();

        int run(const char* path) const;

    private:
        Context context;
        Allocator* allocator = nullptr;
        lua_State* state = nullptr;
    };

}