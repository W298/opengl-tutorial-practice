#ifndef INPUT_HPP
#define INPUT_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

void computeMatricesFromInputs(GLFWwindow* window, mat4& projMat, mat4& viewMat);

#endif
