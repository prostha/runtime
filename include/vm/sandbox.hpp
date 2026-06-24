#pragma once


#include "allocator.hpp"
#include <cstddef>

struct lua_State;

namespace sandbox {

    constexpr size_t MEMORY = 32 * 1024 * 1024;
    constexpr int INSTRUCTIONS = 100000;

    class Sandbox {
    public:
        Sandbox();
        ~Sandbox();

        Sandbox(const Sandbox&) = delete;
        Sandbox& operator=(const Sandbox&) = delete;

        int run(const char* path) const;

    private:
        lua_State* state = nullptr;
        allocator::Allocator* memory = nullptr;
    };

}