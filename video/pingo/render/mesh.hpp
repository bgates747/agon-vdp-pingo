#pragma once

#include <cstdint>

#include "../math/vec2.hpp"
#include "../math/vec3.hpp"

namespace p3d {
struct Mesh {
    int indexes_count;
    uint16_t * pos_indices;
    Vec3f * positions;
};

} // namespace p3d