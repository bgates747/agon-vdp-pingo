#include "object.hpp"

namespace p3d {

Renderable object_as_renderable(Object * object){
    return (Renderable){.renderableType = RENDERABLE_OBJECT, .impl = object};
}

} // namespace p3d