#include "transformable.hpp" 

namespace p3d {

void initialize_scale(Transformable& t) {
    t.scale.x = 1.0f;
    t.scale.y = 1.0f;
    t.scale.z = 1.0f;
    t.modified = true;
}

void initialize(Transformable& t) {
    std::memset(&t, 0, sizeof(Transformable));
    initialize_scale(t);
}

// Helper function to get the forward direction of the transformable
Vec3f get_forward_direction(const Transformable& t) {
    Mat4 rotation_matrix = mat4Identity();
    if (t.rotation.x) {
        auto r = mat4RotateX(t.rotation.x);
        rotation_matrix = mat4MultiplyM(&rotation_matrix, &r);
    }
    if (t.rotation.y) {
        auto r = mat4RotateY(t.rotation.y);
        rotation_matrix = mat4MultiplyM(&rotation_matrix, &r);
    }
    if (t.rotation.z) {
        auto r = mat4RotateZ(t.rotation.z);
        rotation_matrix = mat4MultiplyM(&rotation_matrix, &r);
    }

    // Forward direction is typically -Z in a right-handed coordinate system
    Vec3f forward = {0.0f, 0.0f, -1.0f};
    return mat4MultiplyVec3(&forward, &rotation_matrix);
}

void compute_transformation_matrix(Transformable& t) {
    t.transform = mat4Scale(t.scale);
    if (t.is_camera) {
        if (t.rotation.x) {
            auto r = mat4RotateX(t.rotation.x);
            t.transform = mat4MultiplyM(&r, &t.transform); // arguments reversed
        }
        if (t.rotation.y) {
            auto r = mat4RotateY(t.rotation.y);
            t.transform = mat4MultiplyM(&r, &t.transform); // arguments reversed
        }
        if (t.rotation.z) {
            auto r = mat4RotateZ(t.rotation.z);
            t.transform = mat4MultiplyM(&r, &t.transform); // arguments reversed
        }
        if (t.translation.x || t.translation.y || t.translation.z) {
            auto trans = mat4Translate(t.translation);
            t.transform = mat4MultiplyM(&trans, &t.transform); // arguments reversed
        }

    } else {
        if (t.rotation.x) {
            auto r = mat4RotateX(t.rotation.x);
            t.transform = mat4MultiplyM(&t.transform, &r);
        }
        if (t.rotation.y) {
            auto r = mat4RotateY(t.rotation.y);
            t.transform = mat4MultiplyM(&t.transform, &r);
        }
        if (t.rotation.z) {
            auto r = mat4RotateZ(t.rotation.z);
            t.transform = mat4MultiplyM(&t.transform, &r);
        }
        if (t.translation.x || t.translation.y || t.translation.z) {
            auto trans = mat4Translate(t.translation);
            t.transform = mat4MultiplyM(&t.transform, &trans);
        }
    }

    t.modified = false;
}

void compute_transformation_matrix_local(Transformable& t) {
    // Initialize the local transformation matrix
    Mat4 transforloc = mat4Scale(t.scale);

    if (t.is_camera) {
        if (t.rotation_loc.x) {
            auto r = mat4RotateX(t.rotation_loc.x);
            transforloc = mat4MultiplyM(&r, &transforloc); // arguments reversed
        }
        if (t.rotation_loc.y) {
            auto r = mat4RotateY(t.rotation_loc.y);
            transforloc = mat4MultiplyM(&r, &transforloc); // arguments reversed
        }
        if (t.rotation_loc.z) {
            auto r = mat4RotateZ(t.rotation_loc.z);
            transforloc = mat4MultiplyM(&r, &transforloc); // arguments reversed
        }
        if (t.translation_loc.x || t.translation_loc.y || t.translation_loc.z) {
            auto trans = mat4Translate(t.translation_loc);
            transforloc = mat4MultiplyM(&trans, &transforloc); // arguments reversed
        }
        // Apply the local transformation matrix to the initial transform
        t.transform = mat4MultiplyM(&t.transform, &transforloc); // arguments reversed

    } else {
        if (t.rotation_loc.x) {
            auto r = mat4RotateX(t.rotation_loc.x);
            transforloc = mat4MultiplyM(&transforloc, &r);
        }
        if (t.rotation_loc.y) {
            auto r = mat4RotateY(t.rotation_loc.y);
            transforloc = mat4MultiplyM(&transforloc, &r);
        }
        if (t.rotation_loc.z) {
            auto r = mat4RotateZ(t.rotation_loc.z);
            transforloc = mat4MultiplyM(&transforloc, &r);
        }
        if (t.translation_loc.x || t.translation_loc.y || t.translation_loc.z) {
            auto trans = mat4Translate(t.translation_loc);
            transforloc = mat4MultiplyM(&transforloc, &trans);
        }
        // Apply the local transformation matrix to the initial transform
        t.transform = mat4MultiplyM(&transforloc, &t.transform);
    }

    // Clear local transformation values
    t.rotation_loc = {0.0f, 0.0f, 0.0f};
    t.translation_loc = {0.0f, 0.0f, 0.0f};

    t.modified_loc = false;
}

void dump(const Transformable& t) {
    for (int i = 0; i < 16; i++) {
        std::printf("        [%i] %f\n", i, t.transform.elements[i]);
    }
    std::printf("Scale:       %f %f %f\n", t.scale.x, t.scale.y, t.scale.z);
    std::printf("Rotation:    %f %f %f\n", t.rotation.x * (180.0 / M_PI), t.rotation.y * (180.0 / M_PI), t.rotation.z * (180.0 / M_PI));
    std::printf("Translation: %f %f %f\n", t.translation.x, t.translation.y, t.translation.z);
}

} // namespace p3d
