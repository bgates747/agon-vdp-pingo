#include "renderable.hpp"
#include "renderer.hpp"

namespace p3d {
    
int (*renderingFunctions[RENDERABLE_COUNT])(Mat4 transform, Renderer *, Renderable);

} // namespace p3d