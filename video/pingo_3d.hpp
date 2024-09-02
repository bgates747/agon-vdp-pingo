#ifndef PINGO_3D_H
#define PINGO_3D_H

#include <cstdint>
#include <string>
#include <agon.h>
#include <map>
#include "esp_heap_caps.h"
#include "sprites.h"
#include "vdu_stream_processor.h"

#include "pingo/math/helpers.hpp"
#include "pingo/render/mesh.hpp"
#include "pingo/render/object.hpp"
#include "pingo/render/transformable.hpp"
#include "pingo/render/camera.hpp"
#include "pingo/render/renderer.hpp"
#include "pingo/render/scene.hpp"
#include "pingo/render/depth.hpp"

// DEBUG ONLY
#include "pingo/test.hpp"

#define PINGO_3D_CONTROL_TAG    0x43443350 // "P3DC"

#define PI2                    6.283185307179586476925286766559f

struct P3DCtl {
    uint32_t            m_tag;                  // Used to verify the existence of this structure
    uint32_t            m_size;                 // Used to verify the existence of this structure
    VDUStreamProcessor* m_proc;                 // Used by subcommands to obtain more data
    uint16_t            m_width;                // Width of final render in pixels
    uint16_t            m_height;               // Height of final render in pixels
    p3d::Scene          m_scene;                // Scene transformation settings
    p3d::Camera         m_camera;               // Camera transformation settings
    p3d::Renderer       m_renderer;             // Renderer settings
    std::map<uint16_t, p3d::Mesh>* m_meshes;    // Map of meshes for use by objects
    std::map<uint16_t, p3d::TexObject>* m_objects; // Map of textured objects that use meshes and have transforms
    uint8_t             m_dither_type;          // Dithering type and options to be applied to rendered bitmap

    uint8_t             m_lastc;            // DEBUG: Last color used for screen coloring

bool validate() {
    return (m_tag == PINGO_3D_CONTROL_TAG && m_size == sizeof(P3DCtl));
}

void show_free_ram() {
    printf("Free PSRAM: %u\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void show_ram_used() {
    printf("RAM used by control structure: %u bytes\n", sizeof(P3DCtl));
}

// VDU 23, 0, &A0, sid; &49, 0, 1 :  Initialize Control Structure
void initialize(VDUStreamProcessor& processor, uint16_t w, uint16_t h) {
    printf("initialize: pingo creating control structure for %ux%u scene\n", w, h);
    memset(this, 0, sizeof(P3DCtl));
    m_tag = PINGO_3D_CONTROL_TAG;
    m_size = sizeof(P3DCtl);
    m_proc = &processor;
    m_width = w;
    m_height = h;
    sceneInit(&m_scene);

    float fieldOfView = 1.5708f;                     // 90 degrees field of view in radians
    float aspectRatio = m_width / m_height;          // Aspect ratio (width/height)
    float nearClippingPlane = 1.0f;                  // Near clipping plane
    float farClippingPlane = 2500.0f;                // Far clipping plane
    p3d::Camera m_camera(p3d::mat4Identity(), fieldOfView, aspectRatio, nearClippingPlane, farClippingPlane);

    fabgl::RGBA2222 clearColor = {0xFF};
    p3d::Renderer renderer(&m_scene, &m_camera, m_width, m_width, clearColor, 1);

    m_meshes = new std::map<uint16_t, p3d::Mesh>;
    m_objects = new std::map<uint16_t, p3d::TexObject>;

// DEBUG TESTING STUFF
    m_lastc = 0;
    // p3d::test_vec2f_operations();
    // p3d::test_vec2i_operations();
    // p3d::test_vec3f_operations();
    // p3d::test_vec3i_operations();
}

void handle_subcommand(VDUStreamProcessor& processor, uint8_t subcmd) {
    debug_log("P3D: handle_subcommand(%hu)\n", subcmd);
    m_proc = &processor;
    switch (subcmd) {
            case 1: define_mesh_vertices(); break;
            case 2: set_mesh_vertex_indexes(); break;
            case 3: define_object_texture_coordinates(); break;
            case 4: set_object_texture_coordinate_indexes(); break;
            case 5: create_object(); break;
        case 255: run_tests(); break;
    }
}

void color_screen() {
    auto bitmap = getBitmap(257);  
    uint8_t c = m_lastc;
    m_lastc++;
    for (int y = 0; y < m_height; y++) {
        for (int x = 0; x < m_width; x++) {
            c |= 0b11000000;
            bitmap->setPixel(x, y, fabgl::RGBA2222(c));
            c++;
        }
    }
}

void run_tests() {
    // Step 1: Color the screen
    auto start = millis();
    color_screen();
    auto end = millis();
    auto elapsed_color = end - start;
    
    // Calculate FPS for coloring
    float fps_color = 1000.0f / elapsed_color;

    // Step 2: Retrieve the bitmap and draw it on the canvas
    start = millis();
    auto bitmap = getBitmap(257);  

    const fabgl::Bitmap* rawBitmapPtr = bitmap.get();
    canvas->drawBitmap(0, 0, rawBitmapPtr);
    end = millis();
    auto elapsed_draw = end - start;

    // Calculate FPS for drawing
    float fps_draw = 1000.0f / elapsed_draw;

    // Total elapsed time
    auto total_elapsed = elapsed_color + elapsed_draw;
    float fps_total = 1000.0f / total_elapsed;

    // Print the results
    printf("Time (ms), FPS: color_screen: %u, %.2f; draw: %u, %.2f; total: %u, %.2f\n",
           elapsed_color, fps_color, elapsed_draw, fps_draw, total_elapsed, fps_total);
}

p3d::Mesh* establish_mesh(uint16_t mid) {
    auto mesh_iter = m_meshes->find(mid);
    if (mesh_iter == m_meshes->end()) {
        p3d::Mesh mesh;
        memset(&mesh, 0, sizeof(mesh));
        (*m_meshes).insert(std::pair<uint16_t, p3d::Mesh>(mid, mesh));
        return &m_meshes->find(mid)->second;
    } else {
        return &mesh_iter->second;
    }
}

p3d::Mesh* get_mesh(int32_t mid) {
    if (mid >= 0) {
        return establish_mesh(mid);
    }
    return NULL;
}

// Establish a TexObject with the given ID
p3d::TexObject* establish_object(uint16_t oid) {
    auto object_iter = m_objects->find(oid);
    if (object_iter == m_objects->end()) {
        p3d::TexObject object(oid, nullptr, nullptr, nullptr, nullptr);
        memset(&object, 0, sizeof(object));  // Ensure the object is zero-initialized
        (*m_objects).insert(std::pair<uint16_t, p3d::TexObject>(oid, object));
        return &m_objects->find(oid)->second;
    } else {
        return &object_iter->second;
    }
}

// Retrieve a TexObject from the input
p3d::TexObject* get_object(uint16_t oid) {
    if (oid >= 0) {
        return establish_object(oid);
    }
    return nullptr;
}

// VDU 23, 0, &A0, sid; &49, 1, mid; n; x0; y0; z0; ... :  Define Mesh Vertices
void define_mesh_vertices() {
    auto mid = m_proc->readWord_t();
    auto mesh = get_mesh(mid);
    if (mesh && mesh->positions) {
        delete[] mesh->positions;
        mesh->positions = nullptr;
    }
    auto n = static_cast<uint32_t>(m_proc->readWord_t());
    if (n > 0) {
        mesh->positions = new p3d::Vec3f[n];
        if (!mesh->positions) {
            debug_log("define_mesh_vertices: failed to allocate %u vertices\n", n);
            show_free_ram();
            return;
        }
        debug_log("Reading %u vertices\n", n);
        for (uint32_t i = 0; i < n; i++) {
            uint16_t x = m_proc->readWord_t();
            uint16_t y = m_proc->readWord_t();
            uint16_t z = m_proc->readWord_t();
            mesh->positions[i] = {p3d::convert_position_value(x), p3d::convert_position_value(y), p3d::convert_position_value(z)};
            if (!(i & 0x1F)) debug_log("%u %f %f %f\n", i, mesh->positions[i].x, mesh->positions[i].y, mesh->positions[i].z);
        }
        debug_log("\n");
        printf("Mesh %u: %u vertices\n", mid, n);
    }
}

// VDU 23, 0, &A0, sid; &49, 2, mid; n; i0; ... :  Set Mesh Vertex Indexes
void set_mesh_vertex_indexes() {
    auto mid = m_proc->readWord_t();
    auto mesh = get_mesh(mid);
    if (mesh && mesh->pos_indices) {
        delete[] mesh->pos_indices;
        mesh->pos_indices = nullptr;
        mesh->indexes_count = 0;
    }
    auto n = static_cast<uint32_t>(m_proc->readWord_t());
    if (n > 0) {
        mesh->pos_indices = new uint16_t[n];
        mesh->indexes_count = n;
        if (!mesh->pos_indices) {
            debug_log("set_mesh_vertex_indexes: failed to allocate %u indexes\n", n);
            show_free_ram();
            return;
        }
        debug_log("Reading %u vertex indexes\n", n);
        for (uint32_t i = 0; i < n; i++) {
            mesh->pos_indices[i] = m_proc->readWord_t();
            if (!(i & 0x1F)) debug_log("%u %hu\n", i, mesh->pos_indices[i]);
        }
        debug_log("\n");
        printf("Mesh %u: %u indexes\n", mid, n);
    }
}

// VDU 23, 0, &A0, sid; &49, 3, oid; n; u0; v0; ... :  Define Object Texture Coordinates
void define_object_texture_coordinates() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    if (object && object->textCoord) {
        delete[] object->textCoord;
        object->textCoord = nullptr;
    }
    auto n = static_cast<uint32_t>(m_proc->readWord_t());
    if (n > 0) {
        object->textCoord = new p3d::Vec2f[n];
        if (!object->textCoord) {
            debug_log("define_object_texture_coordinates: failed to allocate %u coordinates\n", n);
            show_free_ram();
            return;
        }
        debug_log("Reading %u texture coordinates\n", n);
        for (uint32_t i = 0; i < n; i++) {
            uint16_t u = m_proc->readWord_t();
            uint16_t v = m_proc->readWord_t();
            object->textCoord[i] = {p3d::convert_texture_coordinate_value(u), 1 - p3d::convert_texture_coordinate_value(v)};
        }
        printf("Object %u: %u texture coordinates\n", oid, n);
    }
}

// VDU 23, 0, &A0, sid; &49, 4, mid; n; i0; ... :  Set Object Texture Coordinate Indexes
void set_object_texture_coordinate_indexes() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    if (object && object->tex_indices) {
        delete[] object->tex_indices;
        object->tex_indices = nullptr;
    }
    auto n = static_cast<uint32_t>(m_proc->readWord_t());
    if (n > 0) {
        object->tex_indices = new uint16_t[n];
        if (!object->tex_indices) {
            debug_log("set_object_texture_coordinate_indexes: failed to allocate %u indexes\n", n);
            show_free_ram();
            return;
        }
        debug_log("Reading %u texture coordinate indexes\n", n);
        for (uint32_t i = 0; i < n; i++) {
            object->tex_indices[i] = m_proc->readWord_t();
            if (!(i & 0x1F)) debug_log("%u %hu\n", i, object->tex_indices[i]);
        }
        printf("Object %u: %u texture coordinate indexes\n", oid, n);
    }
}

// VDU 23, 0, &A0, sid; &49, 5, oid; mid; bmid; :  Create Object
void create_object() {
    auto oid = m_proc->readWord_t();
    auto mid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto mesh = get_mesh(mid);
    auto bmid = m_proc->readWord_t();
    if (object && mesh && bmid) {
        debug_log("Creating 3D object %u with bitmap %u\n", object->oid, bmid);
        auto stored_bitmap = getBitmap(bmid);
        if (stored_bitmap) {
            object->mesh = mesh;
            object->texture = stored_bitmap.get();
        }
        printf("Object %u: Mesh %u, Bitmap %u\n", oid, mid, bmid);
    }
}

// VDU 23, 0, &A0, sid; &49, 6, oid; scalex; :  Set Object X Scale Factor
void set_object_x_scale_factor() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto value = m_proc->readWord_t();
    if (object && (value >= 0)) {
        object->scale.x = p3d::convert_scale_value(value);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 7, oid; scaley; :  Set Object Y Scale Factor
void set_object_y_scale_factor() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto value = m_proc->readWord_t();
    if (object && (value >= 0)) {
        object->scale.y = p3d::convert_scale_value(value);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 8, oid; scalez; :  Set Object Z Scale Factor
void set_object_z_scale_factor() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto value = m_proc->readWord_t();
    if (object && (value >= 0)) {
        object->scale.z = p3d::convert_scale_value(value);  // Corrected to use 'z' instead of 'y'
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 9, oid; scalex; scaley; scalez :  Set Object XYZ Scale Factors
void set_object_xyz_scale_factors() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto valuex = m_proc->readWord_t();
    auto valuey = m_proc->readWord_t();
    auto valuez = m_proc->readWord_t();
    if (object && (valuex >= 0) && (valuey >= 0) && (valuez >= 0)) {
        object->scale.x = p3d::convert_scale_value(valuex);
        object->scale.y = p3d::convert_scale_value(valuey);
        object->scale.z = p3d::convert_scale_value(valuez);
        object->modified = true;
    }
}
// VDU 23, 0, &A0, sid; &49, 10, oid; anglex; :  Set Object X Rotation Angle
void set_object_x_rotation_angle() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto value = m_proc->readWord_t();
    if (object) {
        object->rotation.x = p3d::convert_rotation_value(value);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 11, oid; angley; :  Set Object Y Rotation Angle
void set_object_y_rotation_angle() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto value = m_proc->readWord_t();
    if (object) {
        object->rotation.y = p3d::convert_rotation_value(value);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 12, oid; anglez; :  Set Object Z Rotation Angle
void set_object_z_rotation_angle() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto value = m_proc->readWord_t();
    if (object) {
        object->rotation.z = p3d::convert_rotation_value(value);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 13, oid; anglex; angley; anglez; :  Set Object XYZ Rotation Angles
void set_object_xyz_rotation_angles() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto valuex = m_proc->readWord_t();
    auto valuey = m_proc->readWord_t();
    auto valuez = m_proc->readWord_t();
    if (object) {
        object->rotation.x = p3d::convert_rotation_value(valuex);
        object->rotation.y = p3d::convert_rotation_value(valuey);
        object->rotation.z = p3d::convert_rotation_value(valuez);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 141, oid; anglex; angley; anglez; :  Set Object XYZ Rotation Angles Local
void set_object_xyz_rotation_angles_local() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto valuex = m_proc->readWord_t();
    auto valuey = m_proc->readWord_t();
    auto valuez = m_proc->readWord_t();
    if (object) {
        object->rotation_loc.x = p3d::convert_rotation_value(valuex);
        object->rotation_loc.y = p3d::convert_rotation_value(valuey);
        object->rotation_loc.z = p3d::convert_rotation_value(valuez);
        object->modified_loc = true;
    }
}
// VDU 23, 0, &A0, sid; &49, 14, oid; distx; :  Set Object X Translation Distance
void set_object_x_translation_distance() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto value = m_proc->readWord_t();
    if (object) {
        object->translation.x = p3d::convert_translation_value(value);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 15, oid; disty; :  Set Object Y Translation Distance
void set_object_y_translation_distance() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto value = m_proc->readWord_t();
    if (object) {
        object->translation.y = p3d::convert_translation_value(value);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 16, oid; distz; :  Set Object Z Translation Distance
void set_object_z_translation_distance() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto value = m_proc->readWord_t();
    if (object) {
        object->translation.z = p3d::convert_translation_value(value);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 17, oid; distx; disty; distz :  Set Object XYZ Translation Distances
void set_object_xyz_translation_distances() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto valuex = m_proc->readWord_t();
    auto valuey = m_proc->readWord_t();
    auto valuez = m_proc->readWord_t();
    if (object) {
        object->translation.x = p3d::convert_translation_value(valuex);
        object->translation.y = p3d::convert_translation_value(valuey);
        object->translation.z = p3d::convert_translation_value(valuez);
        object->modified = true;
    }
}

// VDU 23, 0, &A0, sid; &49, 145, oid; distx; disty; distz :  Set Object XYZ Translation Distances Local
void set_object_xyz_translation_distances_local() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    auto valuex = m_proc->readWord_t();
    auto valuey = m_proc->readWord_t();
    auto valuez = m_proc->readWord_t();
    if (object) {
        object->translation_loc.x = p3d::convert_translation_value(valuex);
        object->translation_loc.y = p3d::convert_translation_value(valuey);
        object->translation_loc.z = p3d::convert_translation_value(valuez);
        object->modified_loc = true;
    }
}
// VDU 23, 0, &A0, sid; &49, 18, anglex; :  Set Camera X Rotation Angle
void set_camera_x_rotation_angle() {
    auto value = m_proc->readWord_t();
    m_camera.rotation.x = p3d::convert_rotation_value(value);
    m_camera.modified = true;
}

// VDU 23, 0, &A0, sid; &49, 19, angley; :  Set Camera Y Rotation Angle
void set_camera_y_rotation_angle() {
    auto value = m_proc->readWord_t();
    m_camera.rotation.y = p3d::convert_rotation_value(value);
    m_camera.modified = true;
}

// VDU 23, 0, &A0, sid; &49, 20, anglez; :  Set Camera Z Rotation Angle
void set_camera_z_rotation_angle() {
    auto value = m_proc->readWord_t();
    m_camera.rotation.z = p3d::convert_rotation_value(value);
    m_camera.modified = true;
}

// VDU 23, 0, &A0, sid; &49, 21, anglex; angley; anglez; :  Set Camera XYZ Rotation Angles
void set_camera_xyz_rotation_angles() {
    auto valuex = m_proc->readWord_t();
    auto valuey = m_proc->readWord_t();
    auto valuez = m_proc->readWord_t();
    m_camera.rotation.x = p3d::convert_rotation_value(valuex);
    m_camera.rotation.y = p3d::convert_rotation_value(valuey);
    m_camera.rotation.z = p3d::convert_rotation_value(valuez);
    m_camera.modified = true;
}

// VDU 23, 0, &A0, sid; &49, 149, anglex; angley; anglez; :  Set Camera XYZ Rotation Angles Local
void set_camera_xyz_rotation_angles_local() {
    auto valuex = m_proc->readWord_t();
    auto valuey = m_proc->readWord_t();
    auto valuez = m_proc->readWord_t();
    m_camera.rotation_loc.x = p3d::convert_rotation_value(valuex);
    m_camera.rotation_loc.y = p3d::convert_rotation_value(valuey);
    m_camera.rotation_loc.z = p3d::convert_rotation_value(valuez);
    m_camera.modified_loc = true;
}
// VDU 23, 0, &A0, sid; &49, 42, oid; : Rotate Camera to track a specified object
void camera_track_object() {
    auto oid = m_proc->readWord_t();
    auto object = get_object(oid);
    if (object) {
        // Extract camera position from the camera's transformation matrix (row-major order)
        p3d::Vec3f camera_position = { 
            m_camera.transform.elements[3],  // x
            m_camera.transform.elements[7],  // y
            m_camera.transform.elements[11]  // z
        };

        // Extract object position from the object's transformation matrix (row-major order)
        p3d::Vec3f object_position = { 
            object->transform.elements[3],   // x
            object->transform.elements[7],   // y
            object->transform.elements[11]   // z
        };

        // Calculate the direction vector from the camera to the object
        p3d::Vec2f direction_to_object = {
            object_position.x - camera_position.x,
            object_position.z - camera_position.z  // Mapping z to y in Vec2f
        };

        // Normalize the direction vector
        float length = sqrt(
            direction_to_object.x * direction_to_object.x +
            direction_to_object.y * direction_to_object.y
        );
        if (length > 0) {
            direction_to_object.x /= length;
            direction_to_object.y /= length;
        }

        // Calculate yaw (rotation around the Y axis)
        float yaw = atan2(direction_to_object.x, -direction_to_object.y);

        m_camera.rotation.y = yaw;
        m_camera.rotation.x = 0;
        m_camera.rotation.z = 0;
        m_camera.modified = true;

        // // Debug output
        // printf("Camera: x=%.2f, y=%.2f, z=%.2f Target: x=%.2f, y=%.2f, z=%.2f Angle: %.2f degrees\n", 
        //     camera_position.x, camera_position.y, camera_position.z, 
        //     object_position.x, object_position.y, object_position.z, 
        //     yaw * (180.0f / M_PI));
    }
}

// VDU 23, 0, &A0, sid; &49, 22, distx; :  Set Camera X Translation Distance
void set_camera_x_translation_distance() {
    auto value = m_proc->readWord_t();
    m_camera.translation.x = p3d::convert_translation_value(value);
    m_camera.modified = true;
}

// VDU 23, 0, &A0, sid; &49, 23, disty; :  Set Camera Y Translation Distance
void set_camera_y_translation_distance() {
    auto value = m_proc->readWord_t();
    m_camera.translation.y = p3d::convert_translation_value(value);
    m_camera.modified = true;
}

// VDU 23, 0, &A0, sid; &49, 24, distz; :  Set Camera Z Translation Distance
void set_camera_z_translation_distance() {
    auto value = m_proc->readWord_t();
    m_camera.translation.z = p3d::convert_translation_value(value);
    m_camera.modified = true;
}

// VDU 23, 0, &A0, sid; &49, 25, distx; disty; distz :  Set Camera XYZ Translation Distances
void set_camera_xyz_translation_distances() {
    auto valuex = m_proc->readWord_t();
    auto valuey = m_proc->readWord_t();
    auto valuez = m_proc->readWord_t();
    m_camera.translation.x = p3d::convert_translation_value(valuex);
    m_camera.translation.y = p3d::convert_translation_value(valuey);
    m_camera.translation.z = p3d::convert_translation_value(valuez);
    m_camera.modified = true;
}

// VDU 23, 0, &A0, sid; &49, 153, distx; disty; distz :  Set Camera XYZ Translation Distances Local
void set_camera_xyz_translation_distances_local() {
    auto valuex = m_proc->readWord_t();
    auto valuey = m_proc->readWord_t();
    auto valuez = m_proc->readWord_t();
    m_camera.translation_loc.x = p3d::convert_translation_value(valuex);
    m_camera.translation_loc.y = p3d::convert_translation_value(valuey);
    m_camera.translation_loc.z = p3d::convert_translation_value(valuez);
    m_camera.modified_loc = true;
}

// // VDU 23, 0, &A0, sid; &49, 26, scalex; :  Set Scene X Scale Factor
// void set_scene_x_scale_factor() {
//     auto value = m_proc->readWord_t();
//     if (value >= 0) {
//         m_scene.scale.x = p3d::convert_scale_value(value);
//         m_scene.modified = true;
//     }
// }

// // VDU 23, 0, &A0, sid; &49, 27, scaley; :  Set Scene Y Scale Factor
// void set_scene_y_scale_factor() {
//     auto value = m_proc->readWord_t();
//     if (value >= 0) {
//         m_scene.scale.y = p3d::convert_scale_value(value);
//         m_scene.modified = true;
//     }
// }

// // VDU 23, 0, &A0, sid; &49, 28, scalez; :  Set Scene Z Scale Factor
// void set_scene_z_scale_factor() {
//     auto value = m_proc->readWord_t();
//     if (value >= 0) {
//         m_scene.scale.z = p3d::convert_scale_value(value);
//         m_scene.modified = true;
//     }
// }

// // VDU 23, 0, &A0, sid; &49, 29, scalex; scaley; scalez :  Set Scene XYZ Scale Factors
// void set_scene_xyz_scale_factors() {
//     auto valuex = m_proc->readWord_t();
//     auto valuey = m_proc->readWord_t();
//     auto valuez = m_proc->readWord_t();
//     if ((valuex >= 0) && (valuey >= 0) && (valuez >= 0)) {
//         m_scene.scale.x = p3d::convert_scale_value(valuex);
//         m_scene.scale.y = p3d::convert_scale_value(valuey);
//         m_scene.scale.z = p3d::convert_scale_value(valuez);
//         m_scene.modified = true;
//     }
// }

// // VDU 23, 0, &A0, sid; &49, 30, anglex; :  Set Scene X Rotation Angle
// void set_scene_x_rotation_angle() {
//     auto value = m_proc->readWord_t();
//     m_scene.rotation.x = p3d::convert_rotation_value(value);
//     m_scene.modified = true;
// }

// // VDU 23, 0, &A0, sid; &49, 31, angley; :  Set Scene Y Rotation Angle
// void set_scene_y_rotation_angle() {
//     auto value = m_proc->readWord_t();
//     m_scene.rotation.y = p3d::convert_rotation_value(value);
//     m_scene.modified = true;
// }

// // VDU 23, 0, &A0, sid; &49, 32, anglez; :  Set Scene Z Rotation Angle
// void set_scene_z_rotation_angle() {
//     auto value = m_proc->readWord_t();
//     m_scene.rotation.z = p3d::convert_rotation_value(value);
//     m_scene.modified = true;
// }

// // VDU 23, 0, &A0, sid; &49, 33, anglex; angley; anglez; :  Set Scene XYZ Rotation Angles
// void set_scene_xyz_rotation_angles() {
//     auto valuex = m_proc->readWord_t();
//     auto valuey = m_proc->readWord_t();
//     auto valuez = m_proc->readWord_t();
//     m_scene.rotation.x = p3d::convert_rotation_value(valuex);
//     m_scene.rotation.y = p3d::convert_rotation_value(valuey);
//     m_scene.rotation.z = p3d::convert_rotation_value(valuez);
//     m_scene.modified = true;
// }

// // VDU 23, 0, &A0, sid; &49, 34, distx; :  Set Scene X Translation Distance
// void set_scene_x_translation_distance() {
//     auto value = m_proc->readWord_t();
//     m_scene.translation.x = p3d::convert_translation_value(value);
//     m_scene.modified = true;
// }

// // VDU 23, 0, &A0, sid; &49, 35, disty; :  Set Scene Y Translation Distance
// void set_scene_y_translation_distance() {
//     auto value = m_proc->readWord_t();
//     m_scene.translation.y = p3d::convert_translation_value(value);
//     m_scene.modified = true;
// }

// // VDU 23, 0, &A0, sid; &49, 36, distz; :  Set Scene Z Translation Distance
// void set_scene_z_translation_distance() {
//     auto value = m_proc->readWord_t();
//     m_scene.translation.z = p3d::convert_translation_value(value);
//     m_scene.modified = true;
// }

// // VDU 23, 0, &A0, sid; &49, 37, distx; disty; distz :  Set Scene XYZ Translation Distances
// void set_scene_xyz_translation_distances() {
//     auto valuex = m_proc->readWord_t();
//     auto valuey = m_proc->readWord_t();
//     auto valuez = m_proc->readWord_t();
//     m_scene.translation.x = p3d::convert_translation_value(valuex);
//     m_scene.translation.y = p3d::convert_translation_value(valuey);
//     m_scene.translation.z = p3d::convert_translation_value(valuez);
//     m_scene.modified = true;
// }

// VDU 23, 0, &A0, sid; &49, 41, type : Set Rendering Dither Type
void set_rendering_dither_type() {
    auto type = m_proc->readByte_t();
    m_dither_type = type & 0x03; // Mask the first two bits
    switch (m_dither_type) {
        case 0:
            debug_log("Dithering disabled\n");
            break;
        case 1:
            debug_log("Dithering enabled (Bayer)\n");
            break;
        case 2:
            debug_log("Dithering enabled (Floyd-Steinberg)\n");
            break;
        default:
            m_dither_type = 0;
            debug_log("Invalid dithering type %u\n", m_dither_type);
            break;
    }
}

// VDU 23, 0, &A0, sid; &49, 38, bmid; :  Render To Bitmap
void render_to_bitmap() {
    auto start = millis();

    auto bmid = m_proc->readWord_t();
    if (bmid < 0) {
        return;
    }

    if (m_camera.modified) {
        p3d::compute_transformation_matrix(m_camera);
    }
    if (m_camera.modified_loc) {
        p3d::compute_transformation_matrix_local(m_camera);
    }
    //debug_log("Camera:\n");
    // m_camera.dump();

    for (auto object = m_objects->begin(); object != m_objects->end(); object++) {
        if (object->second.modified) {
            p3d::compute_transformation_matrix(object->second);
            //object->second.dump();
        }
        if (object->second.modified_loc) {
            p3d::compute_transformation_matrix_local(object->second);
            //object->second.dump();
        }
        
    }

    // if (m_scene.modified) {
    //     m_scene.compute_transformation_matrix();
    // }
    // scene.transform = m_scene.transform;

    //debug_log("Frame data:  %02hX %02hX %02hX %02hX\n", m_frame->r, m_frame->g, m_frame->b, m_frame->a);
    //debug_log("Destination: %02hX %02hX %02hX %02hX\n", dst_pix->r, dst_pix->g, dst_pix->b, dst_pix->a);

    // // Apply dithering to the rendered image before copying to the destination bitmap
    // switch (m_dither_type) {
    //     case 0:
    //         break; // no dithering applied
    //     case 1:
    //         dither_bayer((uint8_t*)m_frame, m_width, m_height);
    //         break;
    //     case 2:
    //         dither_floyd_steinberg((uint8_t*)m_frame, m_width, m_height);
    //         break;
    //     default:
    //         m_dither_type = 0; // no dithering applied
    //         debug_log("Invalid dithering type %u\n", m_dither_type);
    //         break;
    // }

    auto stop = millis();
    auto diff = stop - start;
    float fps = 1000.0 / diff;
    // printf("Render to %ux%u took %u ms (%.2f FPS)\n", m_width, m_height, diff, fps);
    //debug_log("Frame data:  %02hX %02hX %02hX %02hX\n", m_frame->r, m_frame->g, m_frame->b, m_frame->a);
    //debug_log("Final data:  %02hX %02hX %02hX %02hX\n", dst_pix->r, dst_pix->g, dst_pix->b, dst_pix->a);
}

}; // struct P3DCtl

#endif // PINGO_3D_H