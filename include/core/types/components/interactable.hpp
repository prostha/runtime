#pragma once

namespace core::components {
    struct Interactable {
        bool hover{false};
        bool press{false};
        bool focus{false};
        bool disable{false};
    };
}