#include <tuple>
#include <iostream>
#include <cstdint>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <chrono>
#include <thread>
#include <GLFW/glfw3.h>
#include "loader.hpp"

// application constans
const uint16_t window_width = 500;
const uint16_t window_height = 500;
const char * window_name = "flow render";
const char * filename = "dump.bin";

// public data
flat_data_t * data;
// data of lines (x1 y1 x2 y2) 
float * grid;
// lines in the grid
uint8_t lines_count = 20;
// application fps
float frame_rate = 30.0f;

// OMG WAT IS THIS DIRTY HACK?
float find_area_radius(flat_data_t * data) {
    float r2 = data->data[0] * data->data[0] + data->data[1] * data->data[1];
    for (uint32_t i = 2; i < 2 * data->frame_count * data->particle_count; i += 2) {
        float _r2 = data->data[i] * data->data[i] + data->data[i+1] * data->data[i+1];
        if (_r2 > r2) { r2 = _r2; }
    }
    return std::sqrt(r2);
}

void error_callback(int error, const char* description) {
    std::cout << "[error]: " << description << " (" << error << ")" << std::endl;
}

/* RENDER PROCEDURE */
void app_render(GLFWwindow * window) {
    static uint32_t select_frame = 0;

    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();

    // draw grid (change the color)
    glColor3f(0.5, 0.5f, 0.5f);
    glPushMatrix();
    glLoadIdentity();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, grid);
    glDrawArrays(GL_LINES, 0, 2 * lines_count);
    glDisableClientState(GL_VERTEX_ARRAY);

    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, grid);
    glDrawArrays(GL_LINES, 0, 2 * lines_count);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();

    // draw points
    glPointSize(5.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    uint32_t current_frame = 2 * select_frame * data->particle_count;

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, data->data + current_frame);
    glDrawArrays(GL_POINTS, 0, data->particle_count);
    glDisableClientState(GL_VERTEX_ARRAY);

    if (select_frame > data->frame_count) {
        select_frame = 0;
    } else {
        select_frame++;
    }

    glfwSwapBuffers(window);
}

void app_sleep(float frame_rate) {
    static double frame_start = 0;
    double wait_time = 1.0 / frame_rate;

    double right_now = glfwGetTime();
    if (right_now - frame_start < wait_time) {
        double dur = (wait_time - (right_now - frame_start));
        // I have no words...
        std::this_thread::sleep_for(std::chrono::milliseconds(long(dur * 1000.0)));
    }
    frame_start = glfwGetTime();
}

float * generate_grid(float r, uint8_t grid_count) {
    const uint8_t coordinate_count = 4;
    float grid_step = 2.0 * r / (float) grid_count;
    float * grid_data = new float [grid_count * coordinate_count];

    for (uint32_t i = 0; i < coordinate_count * grid_count; i += coordinate_count) {
        grid_data[i + 0] = -r + grid_step * (i / coordinate_count);
        grid_data[i + 1] = -r;
        grid_data[i + 2] = -r + grid_step * (i / coordinate_count);
        grid_data[i + 3] = r;
    }

    return grid_data;
}

/* INIT PROCEDURE */
int8_t app_init() {
    data = load_data(filename);
    if (!data) {
        std::cerr << "[error]: " << std::strerror(errno) << " (" << filename << ")" << std::endl;
        return -1;
    }
    // Initialize the library
    if (!glfwInit()) {
        std::cerr << "[error]: couldn't initialize GLFW library" << std::endl;
        return -1;
    }
    glfwSetErrorCallback(error_callback);
    std::cout << "[info]: GLFW " << glfwGetVersionString() << std::endl;
    // Create a windowed mode window and its OpenGL context
    GLFWwindow * window = glfwCreateWindow(window_width, window_height, window_name, NULL, NULL);
    if (!window) {
        std::cerr << "[error]: couldn't create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Make the window's context current
    glfwMakeContextCurrent(window);

    // opengl init block
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // configure opengl view
    glViewport(0, 0, window_width, window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    auto r = find_area_radius(data);
    // set left, right, bottom, top, near, far of viewport
    glOrtho(-r, r, -r, r, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    grid = generate_grid(r, lines_count);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        app_render(window);
        app_sleep(frame_rate);
    }
    glfwTerminate();

    delete[] grid;
    clean_data(data);

    return 0;
}

int main(int argc, char * argv[]) {
    return app_init();
}
