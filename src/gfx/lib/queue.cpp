#include "gfx/lib/queue.hpp"
#include <algorithm>

namespace core::gfx::lib {

    void Queue::sort() noexcept {
        const std::size_t size = keys.size();
        if (size == 0) return;

        sorted.resize(size);
        scratch.resize(size);

        constexpr std::size_t stride = 11;
        constexpr std::size_t buckets = 1 << stride;
        constexpr std::size_t mask = buckets - 1;
        std::size_t histograms[buckets];

        drivers::Key* input = keys.data();
        drivers::Key* output = scratch.data();
        drivers::Command* source = commands.data();
        drivers::Command* target = sorted.data();

        for (std::size_t shift = 0; shift < 64; shift += stride) {
            std::fill_n(histograms, buckets, 0);

            for (std::size_t i = 0; i < size; ++i) {
                histograms[input[i].value >> shift & mask]++;
            }

            std::size_t offset = 0;
            for (std::size_t i = 0; i < buckets; ++i) {
                const std::size_t count = histograms[i];
                histograms[i] = offset;
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

        if (input == scratch.data()) {
            std::copy_n(scratch.begin(), size, keys.begin());
            std::copy_n(sorted.begin(), size, commands.begin());
        }
    }

}