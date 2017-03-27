#include "loader.hpp"

flat_data_t * load_data(const char * filename) {
    FILE * f = fopen(filename, "rb");

    flat_data_t * flat_data = new flat_data_t;

    fread(&(flat_data->particle_count), 1, sizeof(uint32_t), f);
    fread(&(flat_data->frame_count), 1, sizeof(uint32_t), f);

    flat_data->frame = new frame_t [flat_data->frame_count];
    for (uint32_t i = 0; i < flat_data->frame_count; i++) {
        flat_data->frame[i].data = new particle_t [flat_data->particle_count];
        for (uint32_t j = 0; j < flat_data->particle_count; j++) {
            fread(&(flat_data->frame[i].data[j].px), 1, sizeof(float), f);
            fread(&(flat_data->frame[i].data[j].py), 1, sizeof(float), f);
        }
    }

    fclose(f);

    return flat_data;
}

void clean_data(flat_data_t * data) {
    for (uint32_t i = 0; i < data->frame_count; i++) {
        delete[] data->frame[i].data;
    }
    delete[] data->frame;
}