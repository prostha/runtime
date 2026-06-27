#pragma once

#include <vector>

#include "drivers/payload.hpp"

namespace core::gfx::lib {

    class Queue final {
    public:
        Queue() noexcept = default;
        ~Queue() noexcept = default;

        Queue(const Queue&) = delete;
        Queue& operator=(const Queue&) = delete;
        Queue(Queue&&) noexcept = default;
        Queue& operator=(Queue&&) noexcept = delete;

        void push(drivers::Key key, const drivers::Command& command) noexcept;
        drivers::Buffer flush() noexcept;
        void clear() noexcept;
        void sort() noexcept;

    private:
        std::vector<drivers::Key> keys;
        std::vector<drivers::Command> commands;

        std::vector<drivers::Key> scratch;
        std::vector<drivers::Command> sorted;

        drivers::Buffer buffer{};
    };

}