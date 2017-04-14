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
char filename[256] = "dump.bin";

/* LINES BLOCK */
// lines in the grid
const uint8_t lines_count = 20;
// [x1, y1, x2, y2]
const uint8_t coordinate_count = 4;
// vertex to render
const uint32_t vertex_count = lines_count * coordinate_count;
// count of coordinates (2 for horizontal and vertical lines)
const uint32_t elements_count = vertex_count * 2;

// public data
flat_data_t * data;
// data of lines
float * grid;
// application fps
float frame_rate = 30.0f;
// pause
bool pause_flag = false;

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

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        pause_flag = !pause_flag;
    }
}

/* RENDER PROCEDURE */
void app_render(GLFWwindow * window) {
    static uint32_t select_frame = 0;

    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();

    glEnable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw grid (change the color)
    glColor4f(0.5, 0.5f, 0.5f, 0.5f);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, grid);
    glDrawArrays(GL_LINES, 0, vertex_count);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);

    // draw points
    glPointSize(5.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    uint32_t current_frame = 2 * select_frame * data->particle_count;

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, data->data + current_frame);
    glDrawArrays(GL_POINTS, 0, data->particle_count);
    glDisableClientState(GL_VERTEX_ARRAY);

    if (!pause_flag) {
        if (select_frame > data->frame_count) {
            select_frame = 0;
        } else {
            select_frame++;
        }
    }

    glfwSwapBuffers(window);
}

void app_sleep(float frame_rate) {
    static double frame_start = 0;
    double wait_time = 1.0 / frame_rate;

    double right_now = glfwGetTime();
    if (right_now - frame_start < wait_time) {
        double dur = (wait_time - (right_now - frame_start));
        long duration = long(std::round(dur * 1000.0));
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }
    frame_start = glfwGetTime();
}

float * generate_grid(float r) {
    float grid_step = 2.0 * r / (float) lines_count;
    float * grid_data = new float [elements_count];
    uint32_t first_part = elements_count / 2;

    for (uint32_t i = 0; i < first_part; i += coordinate_count) {
        // vertical lines
        grid_data[i + 0] = -r + grid_step * (i / coordinate_count);
        grid_data[i + 1] = -r;
        grid_data[i + 2] = -r + grid_step * (i / coordinate_count);
        grid_data[i + 3] = r;
        // horizontal lines
        grid_data[first_part + i + 0] = -r;
        grid_data[first_part + i + 1] = -r + grid_step * (i / coordinate_count);
        grid_data[first_part + i + 2] = r;
        grid_data[first_part + i + 3] = -r + grid_step * (i / coordinate_count);
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

    std::cout << "[info]: Renderer " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "[info]: OpenGL version supported " << glGetString(GL_VERSION) << std::endl;

    glfwSetErrorCallback(error_callback);
    glfwSetKeyCallback(window, key_callback);

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

    grid = generate_grid(r);

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
    if (argc > 1) {strcpy(filename, argv[1]);}
    return app_init();
}
