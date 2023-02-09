#version 330 core

layout(location = 0) in vec2 vertexPosition_screenspace;
layout(location = 1) in vec2 vertexUV;

uniform vec2 text2D_size; // Uniform Text2D size (Get from main)

out vec2 UV;

void main() {
	// Map vertex position to -1 ~ 1 
	vec2 vertexPosition_mapped = vertexPosition_screenspace - text2D_size / 2;
	vertexPosition_mapped /= (text2D_size / 2);

	gl_Position = vec4(vertexPosition_mapped, 0, 1);
	UV = vertexUV;
}

