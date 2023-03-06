#version 330

uniform sampler2D tex0;
uniform vec3 lightPos;

in vec3 vertexPos;
in vec2 textureCoords;
in vec3 normal;

out vec4 FragColor;

void main() {

	float ambient = 0.1f;

	vec3 lightDir = normalize(lightPos - vertexPos);
	float cosAngle = dot(normal, lightDir);
	float diffuse = max(cosAngle, 0);

	vec3 color = vec3(texture(tex0, textureCoords));
	vec3 res = (ambient + diffuse) * color;
	FragColor = vec4(res, 1.0);
}
