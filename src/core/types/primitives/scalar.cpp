#include "../../../../include/core/types/primitives/scalar.hpp"

namespace core::primitives {

    std::ostream &operator<<(std::ostream &os, const Scalar &scalar) {
        return os << scalar.value;
    }

}