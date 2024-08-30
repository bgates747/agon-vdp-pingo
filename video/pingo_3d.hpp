#ifndef PINGO_3D_H
#define PINGO_3D_H

#include <stdint.h>
#include <string.h>
#include <agon.h>
#include <map>
#include "esp_heap_caps.h"
#include "sprites.h"
#include "vdu_stream_processor.h"

#include "pingo/pingo.hpp"
#include "pingo/test.hpp"

#define PINGO_3D_CONTROL_TAG    0x43443350 // "P3DC"

typedef struct P3DCtl {
    uint32_t            tag;              // Used to verify the existence of this structure
    uint32_t            size;             // Used to verify the existence of this structure
    VDUStreamProcessor* proc;             // Used by subcommands to obtain more data
    uint16_t            width;            // Width of final render in pixels
    uint16_t            height;           // Height of final render in pixels

    void show_free_ram() {
        printf("Free PSRAM: %u\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    }

    void show_ram_used() {
        printf("RAM used by control structure: %u bytes\n", sizeof(P3DCtl));
    }

    // VDU 23, 0, &A0, sid; &49, 0, 1 :  Initialize Control Structure
    void initialize(VDUStreamProcessor& processor, uint16_t width, uint16_t height) {
        printf("initialize: pingo creating control structure for %ux%u scene\n", width, height);
        memset(this, 0, sizeof(P3DCtl));
        tag = PINGO_3D_CONTROL_TAG;
        size = sizeof(P3DCtl);
        width = width;
        height = height;
        show_free_ram();
        show_ram_used();
        // test_vec2f_operations(); // PASSED
        // test_vec2i_operations(); // PASSED
        // test_vec3f_operations(); // RUN BUT NOT CHECKED
        // test_vec3i_operations(); // RUN BUT NOT CHECKED
    }
} P3DCtl;

#endif // PINGO_3D_H