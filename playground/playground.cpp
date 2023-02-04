#include <stdio.h>
#include <vector>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/input.hpp>
#include <common/objloader.hpp>

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

    // Get a handle for MVP uniform in shader
    GLuint matrixID = glGetUniformLocation(programID, "MVP");
    GLuint modelMatrixID = glGetUniformLocation(programID, "V");
    GLuint viewMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for Light Position in shader
    GLuint lightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // Load texture
    GLuint texture = loadDDS("uvmap.dds");
    GLuint textureID = glGetUniformLocation(programID, "myTextureSampler");

    // Load OBJ
    std::vector<vec3> vertices;
    std::vector<vec2> uvs;
    std::vector<vec3> normals;
    if (!loadOBJ("suzanne.obj", vertices, uvs, normals)) {
        printf("Error occurred while loading obj file");
        return -1;
    }

    // Init Vertex Buffer
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);

    // Init UV buffer
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);

    // Init Normal buffer
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3), &normals[0], GL_STATIC_DRAW);

    glEnable(GL_DEPTH_TEST); // Enable Depth test
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE); // Enable Culling

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // Clear color to dark blue

    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(programID); // Use GLSL program

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureID, 0);

        // Set vertex position data
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

        // Set UV data
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            1,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)0
        );

        // Set Normal data
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
            2,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)0
        );

        // Calculate MVP Matrix each frame
        mat4 projMat, viewMat;
        computeMatricesFromInputs(window, projMat, viewMat);
        mat4 modelMat = mat4(1.0f);
        mat4 mvpMat = projMat * viewMat * modelMat;

        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvpMat[0][0]); // Send MVP Matrix to shader
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]);
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]);

        vec3 lightPos = vec3(4, 4, 4);
        glUniform3f(lightID, lightPos.x, lightPos.y, lightPos.z);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // Draw Obj

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    while(glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    // Cleanup
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    glfwTerminate();

    return 0;
}