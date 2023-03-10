#version 330

uniform sampler2D diffuseMap;
uniform sampler2D specularMap;

uniform vec3 lightPos;
uniform vec3 cameraPos;

in vec3 vertexPos;
in vec2 textureCoords;
in vec3 normal;

//in mat3 TBN;

out vec4 FragColor;

void main() {
	vec3 materialAmbient = vec3(1.0, 0.5, 0.31);
	vec3 materialDiffuse = vec3(texture(diffuseMap, textureCoords));
	vec3 materialSpecular = vec3(texture(specularMap, textureCoords));

	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	float ambientStrength = 0.15f;
	vec3 ambient = ambientStrength * materialAmbient;

	vec3 shadeNormal = normalize(normal);
	//vec3 shadeNormal = TBN * normalize(texture(tex0, textureCoords).xyz * 2 - 1);

	vec3 lightDir = normalize(lightPos - vertexPos);
	float cosLightAngle = dot(shadeNormal, lightDir);
	float diffuseStrength = max(cosLightAngle, 0);
	vec3 diffuse = diffuseStrength * materialDiffuse;

	vec3 viewDir = normalize(cameraPos - vertexPos);
	vec3 reflectDir = reflect(-lightDir, shadeNormal);
	float cosViewAngle = dot(viewDir, reflectDir);
	float specularStrength = pow(max(cosViewAngle, 0), 32);
	vec3 specular = specularStrength * materialSpecular;

	vec3 res = (ambient + diffuse + specular) * lightColor;
	FragColor = vec4(res, 1.0);
}
