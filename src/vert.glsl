#version 330

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTextureCoords;
layout (location = 2) in vec3 inNormal;

uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

out vec3 vertexPos;
out vec2 textureCoords;
out vec3 normal;

void main() {
	vertexPos = vec3(model * vec4(inPos, 1.0));
	textureCoords = inTextureCoords;
	normal = vec3(model * vec4(inNormal, 0.0));
	gl_Position = proj * view * model * vec4(inPos, 1.0);
}
