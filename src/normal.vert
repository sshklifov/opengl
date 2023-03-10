#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoords;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

uniform sampler2D normalMap;

out vec3 T;
out vec3 B;
out vec3 N;
out vec3 normal;

void main() {
	vec3 geomNormal = mat3(model) * inNormal;

	vec3 shadeNormal = vec3(texture(normalMap, inTexCoords));
	shadeNormal = normalize(shadeNormal * 2 - 1);

	T = inTangent;
	B = inBitangent;
	N = cross(T, B);

	normal = mat3(model) * mat3(T, B, N) * shadeNormal;
	gl_Position = model * vec4(inPos, 1.0);
}
