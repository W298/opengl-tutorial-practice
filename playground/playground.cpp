#include <stdio.h>
#include <vector>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/input.hpp>

using namespace glm;

bool loadOBJ(const char* path, std::vector<vec3>& outVertices, std::vector<vec2>& outUVs, std::vector<vec3>& outNormals) {
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices; // Index data of vertex, uv, normal
    std::vector<vec3> vertices; // Vertex data
    std::vector<vec2> uvs; // UV data
    std::vector<vec3> normals; // Normal data

    FILE* file = fopen(path, "r");
    if (file == NULL) {
        printf("Unable to open the file \n");
        return false;
    }

    while (true) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF) break;

        if (strcmp(lineHeader, "v") == 0) { // Read vertex line
            vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            vertices.push_back(vertex);
        } else if (strcmp(lineHeader, "vt") == 0) { // Read uv line
            vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uv.y = -uv.y; // Invert V value because DDS format is inverted
            uvs.push_back(uv);
        } else if (strcmp(lineHeader, "vn") == 0) { // Read normal line
            vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            normals.push_back(normal);
        } else if (strcmp(lineHeader, "f") == 0) { // Read face line
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                                 &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                 &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                                 &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9){
                printf("File can't be read\n");
                return false;
            }

            // Insert value to vector
            vertexIndices.insert(vertexIndices.end(), vertexIndex, vertexIndex + 3);
            uvIndices.insert(uvIndices.end(), uvIndex, uvIndex + 3);
            normalIndices.insert(normalIndices.end(), normalIndex, normalIndex + 3);
        }
    }

    // Convert index to data
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        vec3 vertex = vertices[vertexIndices[i] - 1];
        vec2 uv = uvs[uvIndices[i] - 1];
        vec3 normal = normals[normalIndices[i] - 1];

        outVertices.push_back(vertex);
        outUVs.push_back(uv);
        outNormals.push_back(normal);
    }

    fclose(file);
    return true;
}

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

    // Load texture
    GLuint texture = loadDDS("uvmap.DDS");
    GLuint textureID = glGetUniformLocation(programID, "myTextureSampler");

    // Load OBJ
    std::vector<vec3> vertices;
    std::vector<vec2> uvs;
    std::vector<vec3> normals;
    if (!loadOBJ("cube.obj", vertices, uvs, normals)) {
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

        // Calculate MVP Matrix each frame
        mat4 pvMat = computeMatricesFromInputs(window);
        mat4 modelMat = mat4(1.0f);
        mat4 mvpMat = pvMat * modelMat;

        glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvpMat[0][0]); // Send MVP Matrix to shader
        glDrawArrays(GL_TRIANGLES, 0, 12*3); // Draw Cube

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

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