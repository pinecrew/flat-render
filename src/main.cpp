#include <tuple>
#include <iostream>
#include <cstdint>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <GLFW/glfw3.h>
#include "loader.hpp"

// application constans
const uint16_t window_width = 640;
const uint16_t window_height = 480;
const char * window_name = "flow render";
const char * filename = "dump.bin";
const uint16_t max_iterations = 2;

// public data
flat_data_t * data;

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
    static uint16_t iterations = 0;

    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();

    glPointSize(5.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    uint32_t current_frame = 2 * select_frame * data->particle_count;

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, data->data + current_frame);
    glDrawArrays(GL_POINTS, 0, data->particle_count);
    glDisableClientState(GL_VERTEX_ARRAY);

    // my superfast bycicle
    if (iterations >= max_iterations) {
        if (select_frame < data->frame_count) {
            select_frame++;
        } else {
            select_frame = 0;
        }
        iterations = 0;
    } else {
        iterations++;
    }

    glfwSwapBuffers(window);
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

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)) {
        app_render(window);
        /* Poll for and process events */
        glfwPollEvents();
    }
    glfwTerminate();
    clean_data(data);
    return 0;
}

int main(int argc, char * argv[]) {
    return app_init();
}
