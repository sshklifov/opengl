#version 330

uniform sampler2D tex0;
uniform vec3 lightPos;
uniform vec3 cameraPos;

in vec3 vertexPos;
in vec2 textureCoords;
in vec3 normal;

out vec4 FragColor;

void main() {

	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	float ambientStrength = 0.1f;
	vec3 ambient = lightColor * ambientStrength;

	vec3 lightDir = normalize(lightPos - vertexPos);
	float cosLightAngle = dot(normal, lightDir);
	float diffuseStrength = max(cosLightAngle, 0);
	vec3 diffuse = lightColor * diffuseStrength;

	vec3 viewDir = normalize(cameraPos - vertexPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float cosViewAngle = dot(viewDir, reflectDir);
	float specularStrength = pow(max(cosViewAngle, 0), 32);
	vec3 specular = lightColor * specularStrength;

	vec3 objectColor = vec3(texture(tex0, textureCoords));
	vec3 res = (ambient + diffuse + specular) * objectColor;
	FragColor = vec4(res, 1.0);
}
