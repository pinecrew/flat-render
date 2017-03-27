#pragma once
#include <iostream>
#include <cstdint>

struct particle_t {
    float px;
    float py;    
};

struct frame_t {
    particle_t * data;
};

struct flat_data_t {
    frame_t * frame;
    uint32_t particle_count;
    uint32_t frame_count;
};

flat_data_t * load_data(const char * filename);
void clean_data(flat_data_t * data);