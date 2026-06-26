#include "core/primitives/scalar.hpp"

namespace core::ecs::primitives {

    std::ostream &operator<<(std::ostream &os, const Scalar &scalar) {
        return os << scalar.value;
    }

}