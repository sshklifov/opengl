#version 330
in vec2 st;
out vec4 FragColor;
uniform sampler2D tex0;
uniform sampler2D tex1;
in vec3 vertexPos;
void main() {
	vec3 diffuseColor = vec3(texture(tex0, st));
	float ambientLight = 0.1f;
	vec3 ambientColor = diffuseColor * ambientLight;
	FragColor = vec4(ambientColor, 1.0);
	vec3 lightPos = vec3(0, 2, 0);
	vec3 lightDir = vertexPos - lightPos;
	// vec3 normal = ???;
	FragColor = vec4(diffuseColor, 1.0);
}
