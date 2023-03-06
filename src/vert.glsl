#version 330
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTextureCoords;
layout (location = 2) in vec3 aNormal;
uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;
out vec2 st;
out vec3 vertexPos;
void main() {
	vertexPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = proj * view * model * vec4(aPos, 1.0);
	st = aTextureCoords;
}
