#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <GLM/glm.hpp>
#include <cstdio>

#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"
#include "FNL/FastNoiseLite.h"

int main()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(640, 480, "CraftVortex test", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Nie udalo sie utworzyc okna\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Nie udalo sie zaladowac GLAD\n");
        return -1;
    }
    std::printf("OpenGL: %s\n", glGetString(GL_VERSION));

    // Szybkie sprawdzenie pozostalych bibliotek
    glm::vec3 v = glm::normalize(glm::vec3(1.0f, 2.0f, 3.0f));
    FastNoiseLite noise(1337);
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    std::printf("GLM: %.3f  Noise: %.3f\n", v.x, noise.GetNoise(10.0f, 20.0f));

    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.45f, 0.65f, 0.95f, 1.0f);   // niebo
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}