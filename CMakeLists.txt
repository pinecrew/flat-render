cmake_minimum_required(VERSION 2.6)

find_package(OpenGL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)



include_directories(
            ${OPENGL_INCLUDE_DIR}
                    ${GLFW_INCLUDE_DIRS}
)
add_executable(flat-render src/main.cpp src/loader.cpp)
target_compile_features(flat-render PRIVATE cxx_aggregate_default_initializers)
target_link_libraries(flat-render ${OPENGL_gl_LIBRARY} ${GLFW_LIBRARIES})
