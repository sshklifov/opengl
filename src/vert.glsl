#version 330

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTextureCoords;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

out vec3 vertexPos;
out vec2 textureCoords;
out vec3 geomNormal;

out mat3 TBN;

void main() {
	vertexPos = vec3(model * vec4(inPos, 1.0));
	textureCoords = inTextureCoords;
	geomNormal = inNormal;
	gl_Position = proj * view * model * vec4(inPos, 1.0);

	vec3 N = cross(inTangent, inBitangent);
	TBN = mat3(inTangent, inBitangent, N);
}
