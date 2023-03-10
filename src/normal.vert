#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoords;
layout (location = 2) in vec3 inNormal;

uniform mat4 proj;
uniform mat4 model;
uniform mat4 view;

uniform sampler2D normalMap;

out vec3 normal;

void main() {
	//vec3 geomNormal = mat3(model) * inNormal;
	vec3 shadeNormal = vec3(texture(normalMap, inTexCoords));
	shadeNormal = normalize(shadeNormal * 2 - 1);
	normal = shadeNormal;

	gl_Position = model * vec4(inPos, 1.0);
}
