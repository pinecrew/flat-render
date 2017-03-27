#include <tuple>
#include <iostream>
#include <cstdint>
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
std::tuple<float, float, float, float> find_minmax(flat_data_t * data) {
    float min_x = data->frame[0].data[0].px;
    float min_y = data->frame[0].data[0].py;
    float max_x = data->frame[0].data[0].px;
    float max_y = data->frame[0].data[0].py;
    for (uint32_t j = 0; j < data->frame_count; j++) {
        for (uint32_t i = 1; i < data->particle_count; i++) {
            min_x = min_x > data->frame[j].data[i].px ? data->frame[j].data[i].px : min_x;
            min_y = min_y > data->frame[j].data[i].py ? data->frame[j].data[i].py : min_y;
            max_x = max_x < data->frame[j].data[i].px ? data->frame[j].data[i].px : max_x;
            max_y = max_y < data->frame[j].data[i].py ? data->frame[j].data[i].py : max_y;
        }
    }
    return std::make_tuple(min_x, max_x, min_y, max_y);
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
    glBegin(GL_POINTS);
    for (uint32_t i = 0; i < data->particle_count; i++) {
        glVertex2f(data->frame[select_frame].data[i].px, data->frame[select_frame].data[i].py);
    }
    glEnd();

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
    auto mm = find_minmax(data);
    // set left, right, bottom, top, near, far of viewport
    glOrtho(std::get<0>(mm), std::get<1>(mm), std::get<2>(mm), std::get<3>(mm), -1.0f, 1.0f);
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