#include "input.hpp"

mat4 computeMatricesFromInputs(GLFWwindow* window) {
    static double lastTime = glfwGetTime();
    static float initialFOV = 50.0f;
    static float FOV = initialFOV;

    static vec3 position = vec3(4, 4, 0);
    static float angle = 0.0f;

    int width, height;
    double currentTime = glfwGetTime();

    glfwGetWindowSize(window, &width, &height);

    // Return mouse cursor to center
    glfwSetCursorPos(window, width / 2, height / 2);

    // Mouse wheel callback (FOV)
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset) {
        angle += yOffset * 5;
        position.x = 4 * cos(radians(angle));
        position.z = 4 * sin(radians(angle));
    });

    lastTime = currentTime;

    // Calculate matrix
    mat4 projectionMat = perspective(radians(FOV), (float)width / (float)height, 0.1f, 100.0f);
    mat4 viewMat = lookAt(position, vec3(0, 0, 0), vec3(0, 1, 0));

    return projectionMat * viewMat;
}