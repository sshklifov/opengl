#include <cstdio>
#include <cmath>
#include <cstring>
#include <cassert>

#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "../include/glad/glad.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

static void errorCallback(int error, const char* msg) {
	printf("GLFW library error %d: %s\n", error, msg);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, 1);
	}
}

static void resizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

float yaw = 0.f;
float pitch = 0.f;
float xlast = NAN;
float ylast = NAN;
static void mouseCallback(GLFWwindow* window, double xarg, double yarg) {
	const float sensitivity = 0.2f;
	const bool invertY = false;

	float xpos = static_cast<float>(xarg);
	float ypos = static_cast<float>(yarg);
	if (glm::isnan(xlast)) {
		xlast = xpos;
		ylast = ypos;
		return;
	}

	float xoff = xpos - xlast;
	float yoff = ypos - ylast;
	xlast = xpos;
	ylast = ypos;

	yaw += xoff * sensitivity;
	const float yinvert = invertY ? -1.f : 1.f;
	pitch += yinvert * yoff * sensitivity;
	// Avoid gimbal lock
	pitch = glm::clamp(pitch, -89.f, 89.f);
}

unsigned readTexture(const char* imageFile, int width, int height) {
	size_t numPixels = static_cast<size_t>(width * height);
	size_t imageSize = numPixels * 3 * sizeof(unsigned char);
	unsigned char* imageData = static_cast<unsigned char*>(malloc(imageSize));

	FILE * fp = fopen(imageFile, "r");
	size_t numRead = fread(imageData, 1, imageSize, fp);
	assert(!ferror(fp));
	assert(numRead == numPixels*3);
	assert(!feof(fp));
	int c = fgetc(fp);
	assert(c == EOF);
	assert(feof(fp));

	fclose(fp);

	unsigned tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,  GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
	free(imageData);

	return tex;
}

const char* getShaderTypeStr(GLenum type) {
	if (type == GL_VERTEX_SHADER) {
		return "vertex";
	} else if (type == GL_FRAGMENT_SHADER) {
		return "fragment";
	} else {
		return "unknown";
	}
}

unsigned readShader(const char* path, GLenum type) {
	FILE* fp = fopen(path, "r");
	if (!fp) {
		printf("Failed to open shader %s.\n", path);
		return 0;
	}

	const int bufSize = 4096;
	char* buf = static_cast<char*>(malloc(bufSize));
	memset(buf, 0, bufSize);

	fread(buf, 1, bufSize - 1, fp);
	fclose(fp);

	unsigned shader = glCreateShader(type);
	glShaderSource(shader, 1, &buf, NULL);
	glCompileShader(shader);

	int success = 1;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		printf("Problem with compilation of %s shader!\n", getShaderTypeStr(type));
		char info[512];
		glGetShaderInfoLog(shader, 512, NULL, info);
		printf("Shader info log: %s\n", info);
	}

	free(buf);
	return shader;
}

unsigned createShaderProgram(std::initializer_list<unsigned> shaders) {
	unsigned program = glCreateProgram();
	for (unsigned shader : shaders) {
		glAttachShader(program, shader);
	}
	glLinkProgram(program);

	int success = 1;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		printf("Problem with linking of shading program!\n");
	}

	return program;
}

void setProgramUniform(unsigned program, const glm::mat4& mat, const char* name) {
	glUseProgram(program);
	int location = glGetUniformLocation(program, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void setProgramUniform(unsigned program, const glm::vec3& v, const char* name) {
	glUseProgram(program);
	int location = glGetUniformLocation(program, name);
	glUniform3f(location, v[0], v[1], v[2]);
}

void setProgramTexture(unsigned program, unsigned tex, int slot, const char* name) {
	glUseProgram(program);
	GLenum enumSlot = (GLenum)(GL_TEXTURE0 + slot); // Get the correct enum slot
	glActiveTexture(enumSlot);
	glBindTexture(GL_TEXTURE_2D, tex);
	int location = glGetUniformLocation(program, name);
	glUniform1i(location, slot);
}

glm::vec3 calculateNormal(const glm::vec3& pos1, const glm::vec3& pos2, const glm::vec3& pos3) {
	return glm::cross(pos2 - pos1, pos3 - pos2);
}

glm::vec3 calculateTangent(
	const glm::vec3 pos1,
	const glm::vec3 pos2,
	const glm::vec3 pos3,
	const glm::vec2 uv1,
	const glm::vec2 uv2,
	const glm::vec2 uv3
) {
	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1; 
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	glm::vec3 tangent;
	tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	return tangent;
}

glm::vec3 calculateBitangent(
	const glm::vec3 pos1,
	const glm::vec3 pos2,
	const glm::vec3 pos3,
	const glm::vec2 uv1,
	const glm::vec2 uv2,
	const glm::vec2 uv3
) {
	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1; 
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	glm::vec3 bitangent;
	bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
	return bitangent;
}

void appendToVertexBuffer(std::vector<float>& buffer, glm::vec2 v) {
	float* ptr = glm::value_ptr(v);
	buffer.insert(buffer.end(), ptr, ptr + 2);
}

void appendToVertexBuffer(std::vector<float>& buffer, glm::vec3 v) {
	float* ptr = glm::value_ptr(v);
	buffer.insert(buffer.end(), ptr, ptr + 3);
}

unsigned readObjectFile(const char* path, int normalsMode, int& renderCount) {
	FILE* fp = fopen(path, "r");
	if (!fp) {
		printf("Failed to open asset %s.\n", path);
		return 0;
	}

	const int bufSize = 4096;
	char* buf = static_cast<char*>(malloc(bufSize));

	std::vector<glm::vec3> v;
	std::vector<glm::vec2> vt;
	std::vector<glm::vec3> vn;

	std::vector<float> bufferData;
	renderCount = 0;
	while(fgets(buf, 4096, fp)) {
		if (buf[0] == 'v') {
			if (buf[1] == 't') {
				glm::vec2 uv;
				sscanf(buf + 2, "%f %f", &uv.x, &uv.y);
				vt.push_back(uv);
			} else if (buf[1] == 'n') {
				glm::vec3 normal;
				sscanf(buf + 2, "%f %f %f", &normal.x, &normal.y, &normal.z);
				vn.push_back(normal);
			} else if (buf[1] == ' ') {
				glm::vec3 point;
				sscanf(buf + 1, "%f %f %f", &point.x, &point.y, &point.z);
				v.push_back(point);
			}
		} else if (buf[0] == 'f') {
			size_t vi[4] = {0, };
			size_t vti[4] = {0, };
			size_t vni[4] = {0, };
			bool hasNormals = !vn.empty();
			bool quads = false;
			if (hasNormals) {
				int numRead = sscanf(buf + 1, "%lu/%lu/%lu %lu/%lu/%lu %lu/%lu/%lu %lu/%lu/%lu",
					&vi[0], &vti[0], &vni[0], &vi[1], &vti[1], &vni[1], &vi[2], &vti[2], &vni[2], &vi[3], &vti[3], &vni[3]);
				quads = (numRead == 12);
			} else {
				int numRead = sscanf(buf + 1, "%lu/%lu %lu/%lu %lu/%lu %lu/%lu",
					&vi[0], &vti[0], &vi[1], &vti[1], &vi[2], &vti[2], &vi[3], &vti[3]);
				quads = (numRead == 8);
			}
			for (int i = 0; i < 4; ++i) {
				vi[i] -= 1;
				vti[i] -= 1;
				vni[i] -= 1;
			}

			int numTri = quads ? 2 : 1;
			for (int trid = 0; trid < numTri; ++trid) {
				// Offset into vi/vti/vni arrays for current triangle
				int off = 2 * trid;
				// Positions in 3D space + texture coordinates of the triangle.
				// This information is required for tangent space matrix calculation.
				glm::vec3 pos1 = v[vi[off]], pos2 = v[vi[1+off]], pos3 = v[vi[(2+off)%3]];
				glm::vec2 uv1 = vt[vti[off]], uv2 = vt[vti[1+off]], uv3 = vt[vti[(2+off)%3]];
				// Calculate tangent and bitangent vectors
				glm::vec3 tangent = calculateTangent(pos1, pos2, pos3, uv1, uv2, uv3);
				glm::vec3 bitangent = calculateBitangent(pos1, pos2, pos3, uv1, uv2, uv3);
				if (normalsMode) {
					// Write a single arrow for this triangle
					const float oneThird = 1.f / 3.f;
					glm::vec3 point = (pos1 + pos2 + pos3) * oneThird;
					glm::vec2 uv = (uv1 + uv2 + uv3) * oneThird;
					glm::vec3 n(0.f, 0.f, 0.f);
					if (hasNormals) {
						glm::vec3 n1 = vn[vni[off]], n2 = vn[vni[1+off]], n3 = vn[vni[(2+off)%3]];
						n = normalize((n1 + n2 + n3) * oneThird);
					} else {
						n = calculateNormal(pos1, pos2, pos3); // TODO bug fix
					}
					appendToVertexBuffer(bufferData, point);
					appendToVertexBuffer(bufferData, uv);
					appendToVertexBuffer(bufferData, n);
				} else {
					for (int pid = 0; pid < 3; ++pid) {
						// Current index into vi/vti/vni arrays
						int pointIndex = (pid + trid) % 3;
						// Calculate normal
						glm::vec3 n = hasNormals ? vn[vni[pointIndex]] : calculateNormal(pos1, pos2, pos3);
						n = glm::normalize(n);

						appendToVertexBuffer(bufferData, v[vi[pointIndex]]);
						appendToVertexBuffer(bufferData, vt[vti[pointIndex]]);
						appendToVertexBuffer(bufferData, n);
						appendToVertexBuffer(bufferData, tangent);
						appendToVertexBuffer(bufferData, bitangent);
					}
				}
			}

			int renderCountPerTri = normalsMode ? 1 : 3;
			renderCount += renderCountPerTri * numTri;
		}
	}
	free(buf);

	unsigned vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	int bufferSize = static_cast<int>(bufferData.size() * sizeof(float));
	glBufferData(GL_ARRAY_BUFFER, bufferSize, bufferData.data(), GL_STATIC_DRAW);

	if (normalsMode) {
		// Skip tangent and bitangent for now
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(5 * sizeof(float)));
		glEnableVertexAttribArray(2);
	} else {
		// Points, uv coordinates, normals, tangent, bitangent.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(5 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(8 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(11 * sizeof(float)));
		glEnableVertexAttribArray(4);
	}

	return vao;
}

int main() {
	if (!glfwInit()) {
		printf("GLFW failed to init!\n");
		return 1;
	}

	glfwSetErrorCallback(errorCallback);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	int windowWidth = 800;
	int windowHeight = 600;
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Lecture 1", NULL, NULL);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouseCallback);  
	glfwSetKeyCallback(window, keyCallback);

	glfwMakeContextCurrent(window);
	if (!gladLoadGL()) {
		printf("Glad failed to init!\n");
		return 1;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, resizeCallback);

	glEnable(GL_DEPTH_TEST);

	unsigned vertexShader = readShader("src/vert.glsl", GL_VERTEX_SHADER);
	unsigned fragmentShader = readShader("src/frag.glsl", GL_FRAGMENT_SHADER);
	// Shading program for the box light
	unsigned lightVertShader = readShader("src/light.vert", GL_VERTEX_SHADER);
	unsigned lightFragShader = readShader("src/light.frag", GL_FRAGMENT_SHADER);
	// Shading program to display normals
	unsigned normalVertShader = readShader("src/normal.vert", GL_VERTEX_SHADER);
	unsigned normalGeomShader = readShader("src/normal.geom", GL_GEOMETRY_SHADER);
	unsigned normalFragShader = readShader("src/normal.frag", GL_FRAGMENT_SHADER);

	unsigned program = createShaderProgram({vertexShader, fragmentShader});
	unsigned lightProgram = createShaderProgram({lightVertShader, lightFragShader});
	unsigned normalProgram = createShaderProgram({normalVertShader, normalGeomShader, normalFragShader});

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(lightVertShader);
	glDeleteShader(lightFragShader);
	glDeleteShader(normalVertShader);
	glDeleteShader(normalFragShader);

	int renderCountTri = 0;
	unsigned vao = readObjectFile("/home/stef/Downloads/CubeManual.obj", false, renderCountTri);
	int renderCountNormals = 0;
	unsigned normalVao = readObjectFile("/home/stef/Downloads/CubeManual.obj", true, renderCountNormals);

	unsigned diffuseTex = readTexture("/home/stef/Downloads/box_diffuse.rgb", 500, 500);
	unsigned specularTex = readTexture("/home/stef/Downloads/box_specular.rgb", 500, 500);
	unsigned normalTex = readTexture("/home/stef/Downloads/normalmap.rgb", 512, 512);

	const float cameraSpeed = 2.f;
	glm::vec3 cameraPos(0.f, 0.f, 3.f);
	glm::vec3 lightPos(-0.2, 1, 0.7);

	float aspect = (float)windowWidth / (float)windowHeight;
	glm::mat4 proj = glm::perspective(glm::radians(45.f), aspect, 0.1f, 500.f);

	double lastTime = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		double currTime = glfwGetTime();
		float elapsedTime = static_cast<float>(currTime - lastTime);
		lastTime = currTime;

		glfwPollEvents();

		glm::mat4 rot = glm::eulerAngleXY(glm::radians(pitch), glm::radians(yaw));
		glm::vec3 cameraRight(rot[0][0], rot[1][0], rot[2][0]);
		glm::vec3 cameraUp(rot[0][1], rot[1][1], rot[2][1]);
		glm::vec3 cameraForward(rot[0][2], rot[1][2], rot[2][2]);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			cameraPos -= cameraForward * elapsedTime * cameraSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			cameraPos += cameraForward * elapsedTime * cameraSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			cameraPos -= cameraRight * elapsedTime * cameraSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			cameraPos += cameraRight * elapsedTime * cameraSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			cameraPos -= cameraUp * elapsedTime * cameraSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			cameraPos += cameraUp * elapsedTime * cameraSpeed;
		}
		// lightPos = cameraPos;

		float rotAngle = 45.f;
		/* glm::mat4 model = glm::rotate(glm::radians(rotAngle), glm::vec3(1, 1, 1)); */
		glm::mat4 model(1.f);
		glm::mat4 view = rot * glm::translate(-cameraPos);

		glClearColor(0.6f, 0.6f, 0.6f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 1
		glUseProgram(lightProgram);
		glBindVertexArray(vao);

		setProgramUniform(program, proj, "proj");
		setProgramUniform(program, view, "view");
		setProgramUniform(program, model, "model");
		setProgramUniform(program, lightPos, "lightPos");
		setProgramUniform(program, cameraPos, "cameraPos");
		setProgramTexture(program, diffuseTex, 0, "diffuseMap");
		setProgramTexture(program, specularTex, 1, "specularMap");
		setProgramTexture(program, normalTex, 2, "normalMap");
		glDrawArrays(GL_TRIANGLES, 0, renderCountTri);

		// One more time for the light
		glm::mat4 lightModel = glm::translate(lightPos) * glm::scale(glm::vec3(0.1, 0.1, 0.1));
		glUseProgram(lightProgram);
		glBindVertexArray(vao);
		setProgramUniform(lightProgram, proj, "proj");
		setProgramUniform(lightProgram, view, "view");
		setProgramUniform(lightProgram, lightModel, "model");
		glDrawArrays(GL_TRIANGLES, 0, 36);
#endif

#if 1
		// One more time for the normals
		glUseProgram(normalProgram);
		glBindVertexArray(normalVao);
		setProgramUniform(normalProgram, proj, "proj");
		setProgramUniform(normalProgram, view, "view");
		setProgramUniform(normalProgram, model, "model");
		setProgramTexture(normalProgram, normalTex, 0, "normalMap");
		glDrawArrays(GL_POINTS, 0, renderCountNormals);
#endif

		glfwSwapBuffers(window);
	}

	glDeleteProgram(program);
	glDeleteProgram(lightProgram);
	//glDeleteBuffers(1, &vbo);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
