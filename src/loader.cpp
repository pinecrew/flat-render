#include "loader.hpp"

flat_data_t * load_data(const char * filename) {
    FILE * f = fopen(filename, "rb");

    if (f == NULL) {
        return NULL;
    }

    flat_data_t * flat_data = new flat_data_t;

    fread(&(flat_data->particle_count), 1, sizeof(uint32_t), f);
    fread(&(flat_data->frame_count), 1, sizeof(uint32_t), f);

    flat_data->data_count = 2 * flat_data->particle_count * flat_data->frame_count;
    uint32_t readed_data = 0;
    flat_data->data = new float [flat_data->data_count];
    while (flat_data->data_count != readed_data) {
        readed_data += fread(flat_data->data + readed_data, sizeof(float), flat_data->data_count - readed_data, f);
    }

    fclose(f);

    return flat_data;
}

void clean_data(flat_data_t * data) {
    delete[] data->data;
}