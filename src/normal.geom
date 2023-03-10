#version 330 core

layout (points) in;
layout (line_strip, max_vertices = 10) out;

uniform mat4 proj;
uniform mat4 view;

in vec3 normal[];

void main() {    
	const float arrowLen = 0.5f;
	const float arrowHeadLen = 0.1f;
	const float arrowHeadSpread = 0.04f;
	vec3 from = vec3(gl_in[0].gl_Position);

	vec3 to = from + normal[0] * arrowLen;

	gl_Position = proj * view * vec4(from, 1.0);
	EmitVertex();
	gl_Position = proj * view * vec4(to, 1.0);
	EmitVertex();
	EndPrimitive();

	// Compute orthonormal basis using Gram-Schmidt.
	vec3 u1 = normalize(to - from);
	vec3 v2 = vec3(u1.x + u1.y, u1.x - u1.y, u1.z);
	vec3 u2 = normalize(v2 - dot(u1, v2));
	vec3 u3 = cross(u1, u2);
	// Write the arrow head part using the basis vectors u2 and u3
	vec3 center = from * arrowHeadLen + to * (1-arrowHeadLen);

	gl_Position = proj * view * vec4(to, 1.0);
	EmitVertex();
	gl_Position = proj * view * vec4(center + u2 * arrowHeadSpread, 1.0);
	EmitVertex();
	EndPrimitive();

	gl_Position = proj * view * vec4(to, 1.0);
	EmitVertex();
	gl_Position = proj * view * vec4(center - u2 * arrowHeadSpread, 1.0);
	EmitVertex();
	EndPrimitive();

	gl_Position = proj * view * vec4(to, 1.0);
	EmitVertex();
	gl_Position = proj * view * vec4(center + u3 * arrowHeadSpread, 1.0);
	EmitVertex();
	EndPrimitive();

	gl_Position = proj * view * vec4(to, 1.0);
	EmitVertex();
	gl_Position = proj * view * vec4(center - u3 * arrowHeadSpread, 1.0);
	EmitVertex();
	EndPrimitive();
}
