#version 330

uniform sampler2D tex0;
uniform sampler2D tex1;

in vec3 vertexPos;
in vec2 textureCoords;
in vec3 normal;

out vec4 FragColor;

void main() {
	vec3 diffuseColor = vec3(texture(tex0, textureCoords));
	float ambientLight = 0.1f;
	vec3 ambientColor = diffuseColor * ambientLight;
	FragColor = vec4(ambientColor, 1.0);
	vec3 lightPos = vec3(0, 2, 0);
	vec3 lightDir = vertexPos - lightPos;
	// vec3 normal = ???;
	FragColor = vec4(diffuseColor, 1.0);
}
