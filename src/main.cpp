#include <iostream>
#include <cstdint>
#include <GLFW/glfw3.h>
#include "loader.hpp"

// application constans
const uint16_t window_width = 640;
const uint16_t window_height = 480;
const char * window_name = "flow render";

// public data
flat_data_t * data;

void error_callback(int error, const char* description) {
    std::cout << "[error]: " << description << " (" << error << ")" << std::endl;
}

/* RENDER PROCEDURE */
void app_render(GLFWwindow * window) {
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT);
    // some code
    /* Swap front and back buffers */
    glfwSwapBuffers(window);
}

/* INIT PROCEDURE */
int8_t app_init() {
    data = load_data("dump.bin");
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