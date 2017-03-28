#pragma once
#include <iostream>
#include <cstdint>

struct flat_data_t {
    float * data;
    uint32_t particle_count;
    uint32_t frame_count;
    uint32_t data_count;
};

flat_data_t * load_data(const char * filename);
void clean_data(flat_data_t * data);