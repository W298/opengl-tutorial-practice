#include <stdio.h>
#include <vector>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/input.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/text2D.hpp>

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

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); // Get keyboard event
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Disable cursor

    // Create VAO
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Compile GLSL program from shaders
    GLuint programID = LoadShaders("shaders/VertexShader.vert", "shaders/FragmentShader.frag");

    // Get a handle for MVP uniform in shader
    GLuint matrixID = glGetUniformLocation(programID, "MVP");
    GLuint modelMatrixID = glGetUniformLocation(programID, "V");
    GLuint viewMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for Light Position in shader
    GLuint lightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // Load texture
    GLuint texture = loadDDS("Cube.dds");
    GLuint textureID = glGetUniformLocation(programID, "myTextureSampler");

    // Load OBJ
    std::vector<vec3> vertices;
    std::vector<vec2> uvs;
    std::vector<vec3> normals;
    if (!loadOBJ("cube.obj", vertices, uvs, normals)) {
        printf("Error occurred while loading obj file");
        return -1;
    }

    // Load Index
    std::vector<unsigned short> indices;
    std::vector<vec3> indexed_vertices;
    std::vector<vec2> indexed_uvs;
    std::vector<vec3> indexed_normals;
    indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

    // Init Vertex Buffer
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    // Init UV buffer
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(vec2), &indexed_uvs[0], GL_STATIC_DRAW);

    // Init Normal buffer
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(vec3), &indexed_normals[0], GL_STATIC_DRAW);

    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

    initText2D("CascadiaMono.dds", width, height);

    glEnable(GL_DEPTH_TEST); // Enable Depth test
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE); // Enable Culling

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // Clear color to dark blue

    double lastTime = glfwGetTime();
    int nbFrames = 0;
    std::string text;

    do {
        // Print FPS
        double currentTime = glfwGetTime();
        nbFrames++;
        if (currentTime - lastTime >= 1.0) {
            text = std::to_string(nbFrames) + " FPS";
            printf("%s\n", text.c_str());

            nbFrames = 0;
            lastTime += 1.0;
        }

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

        // Set index data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Calculate MVP Matrix each frame
        mat4 projMat, viewMat;
        computeMatricesFromInputs(window, projMat, viewMat);
        mat4 modelMat = scale(mat4(1.0f), vec3(0.2f, 0.2f, 0.2f));
        mat4 mvpMat = projMat * viewMat * modelMat;

        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvpMat[0][0]); // Send MVP Matrix to shader
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMat[0][0]); // Send Model Matrix to shader
        glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMat[0][0]); // Send View Matrix to shader

        vec3 lightPos = vec3(4, 4, 4);
        glUniform3f(lightID, lightPos.x, lightPos.y, lightPos.z); // Send Light position to shader

        // Draw with VBO indexing
        glDrawElements(
            GL_TRIANGLES,
            indices.size(),
            GL_UNSIGNED_SHORT,
            (void*)0
        );

        printText2D(text.c_str(), 50, 50, 50);

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

    cleanupText2D();
    glfwTerminate();

    return 0;
}