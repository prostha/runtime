#include "gfx/lib/queue.hpp"
#include <algorithm>

namespace core::gfx::lib {

void Queue::push(const drivers::Key key, const drivers::Command& command) noexcept {
    this->keys.push_back(key);
    this->commands.push_back(command);
}

drivers::Buffer Queue::flush() noexcept {
    this->sort();
    this->buffer.keys = this->keys.data();
    this->buffer.commands = this->commands.data();
    this->buffer.count = this->keys.size();
    return this->buffer;
}

void Queue::clear() noexcept {
    this->keys.clear();
    this->commands.clear();
}

void Queue::sort() noexcept {
    const std::size_t size = this->keys.size();
    if (size == 0) return;

    this->scratch.resize(size);
    this->sorted.resize(size);

    constexpr std::size_t stride = 11;
    constexpr std::size_t buckets = 1 << stride;
    constexpr std::size_t mask = buckets - 1;
    std::size_t histograms[buckets];

    drivers::Key* input = this->keys.data();
    drivers::Key* output = this->scratch.data();
    drivers::Command* source = this->commands.data();
    drivers::Command* target = this->sorted.data();

    for (std::size_t shift = 0; shift < 64; shift += stride) {
        std::fill_n(histograms, buckets, 0);

        for (std::size_t i = 0; i < size; ++i) {
            histograms[input[i].value >> shift & mask]++;
        }

        std::size_t offset = 0;
        for (unsigned long long & histogram : histograms) {
            const std::size_t count = histogram;
            histogram = offset;
            offset += count;
        }

        for (std::size_t i = 0; i < size; ++i) {
            const std::size_t bucket = input[i].value >> shift & mask;
            const std::size_t destination = histograms[bucket]++;
            output[destination] = input[i];
            target[destination] = source[i];
        }

        std::swap(input, output);
        std::swap(source, target);
    }

    if (input != this->keys.data()) {
        std::copy_n(this->scratch.begin(), size, this->keys.begin());
        std::copy_n(this->sorted.begin(), size, this->commands.begin());
    }
}

}