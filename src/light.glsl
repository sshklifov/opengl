#version 330

uniform sampler2D tex0;

in vec3 vertexPos;
in vec2 textureCoords;
in vec3 normal;

out vec4 FragColor;

void main() {
	FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
