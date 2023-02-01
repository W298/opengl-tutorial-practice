#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
using namespace glm;

int main()
{
    // Init GLFW
    glewExperimental = true;
    if (!glfwInit()) {
        fprintf(stderr, "Failed to Init GLF\\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // Anti-aliasing 4x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // OpenGL 3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MasOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Do not use Old OpenGL

    // Open a window
    GLFWwindow* window;
    float width = 1024.0f;
    float height = 768.0f;
    window = glfwCreateWindow(width, height, "Playground", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window");
        glfwTerminate();
        return -1;
    }

    // Init GLEW
    glfwMakeContextCurrent(window);
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to Init GLEW\n");
        return -1;
    }

    // Set Input
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Create VAO
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Compile GLSL program from shaders
    GLuint programID = LoadShaders("shaders/SimpleVertexShader.vertexshader", "shaders/SimpleFragmentShader.fragmentshader");

    // Create MVP Matrix
    mat4 projectionMat = perspective(radians(45.0f), width / height, 0.1f, 100.0f);
    mat4 viewMat = lookAt(vec3(4, 3, -3), vec3(0, 0, 0), vec3(0, 1, 0));
    mat4 modelMat = mat4(1.0f);

    mat4 translateMat = translate(mat4(), vec3(0, 0, -2));

    mat4 mvpMat = projectionMat * viewMat * modelMat;
    mat4 translatedMVPMat = projectionMat * viewMat * translateMat * modelMat;

    // Get a handle for MVP uniform in shader
    GLuint matrixID = glGetUniformLocation(programID, "MVP");

    // Define Vertex Array (Cube)
    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f,-1.0f,-1.0f, // cube triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // cube triangle 1 : end
        1.0f, 1.0f,-1.0f, // cube triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // cube triangle 2 : end
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, // triangle
        1.0f, -1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    // Define Color for each vertex (Cube)
    static GLfloat g_color_buffer_data[13*3*3];
    for (int v = 0; v < 13*3; v++) {
        g_color_buffer_data[3*v+0] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        g_color_buffer_data[3*v+1] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        g_color_buffer_data[3*v+2] = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    }

    // Init Vertex Buffer
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // Init Color buffer
    GLuint colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // Clear color to dark blue

    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(programID); // Use GLSL program

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            0,          // attr 0 (same with shader)
            3,          // size
            GL_FLOAT,   // type
            GL_FALSE,   // normalized?
            0,          // stride
            (void*)0    // array buffer offset
        );

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)0
        );

        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &translatedMVPMat[0][0]); // Send Translated MVP Matrix to shader
        glDrawArrays(GL_TRIANGLES, 12*3, 3); // Draw Triangle

        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvpMat[0][0]); // Send MVP Matrix to shader
        glDrawArrays(GL_TRIANGLES, 0, 12*3); // Draw Cube

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
}