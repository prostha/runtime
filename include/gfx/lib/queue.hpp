#pragma once
#include "drivers/payload.hpp"
#include <vector>

namespace core::gfx::lib {

    class Queue {
    public:
        void push(const drivers::Key key, const drivers::Command& command) {
            keys.push_back(key);
            commands.push_back(command);
        }

        drivers::Buffer flush() noexcept {
            sort();
            buffer.keys = keys.data();
            buffer.commands = sorted.data();
            buffer.count = keys.size();
            return buffer;
        }

        void clear() noexcept {
            keys.clear();
            commands.clear();
            sorted.clear();
            scratch.clear();
        }

    private:
        void sort() noexcept;

        std::vector<drivers::Key> keys;
        std::vector<drivers::Command> commands;
        std::vector<drivers::Command> sorted;
        std::vector<drivers::Key> scratch;
        drivers::Buffer buffer{};
    };

}