#ifndef PINGO_3D_H
#define PINGO_3D_H

#include <cstdint>
#include <string>
#include <agon.h>
#include <map>
#include "esp_heap_caps.h"
#include "sprites.h"
#include "vdu_stream_processor.h"

#include "pingo/render/mesh.hpp"
#include "pingo/render/object.hpp"
#include "pingo/render/transformable.hpp"
#include "pingo/render/camera.hpp"
#include "pingo/render/renderer.hpp"
#include "pingo/render/scene.hpp"
#include "pingo/render/depth.hpp"
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
    std::map<uint16_t, p3d::Object>* m_objects; // Map of textured objects that use meshes and have transforms
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
    m_objects = new std::map<uint16_t, p3d::Object>;

    // int size = 0;
    // int frame_size = m_width * m_height;
    // m_zeta = p3d::depth_buffer_create(frame_size);

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
            // int idx = (y * m_width) + x;
            // p3d::depth_write(m_zeta, idx, (float) idx);
        }
    }
}

void run_tests() {
    // printf("Starting run_tests...\n");
    
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

}; // struct P3DCtl

#endif // PINGO_3D_H