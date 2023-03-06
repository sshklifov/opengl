#include <cstdio>
#include <cmath>
#include <cstring>
#include <cassert>

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

unsigned createShaderProgram(unsigned vertexShader, unsigned fragmentShader) {
	unsigned program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	int success = 1;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		printf("Problem with linking of shading program!\n");
	}

	return program;
}

void setProgramUni(unsigned program, const glm::mat4& mat, const char* name) {
	glUseProgram(program);
	int location = glGetUniformLocation(program, name);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}

void setProgramUni(unsigned program, const glm::vec3& v, const char* name) {
	glUseProgram(program);
	int location = glGetUniformLocation(program, name);
	glUniform3f(location, v[0], v[1], v[2]);
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
	// Use a different shader for the light
	unsigned lightShader = readShader("src/light.glsl", GL_FRAGMENT_SHADER);

	unsigned program = createShaderProgram(vertexShader, fragmentShader);
	unsigned lightProgram = createShaderProgram(vertexShader, lightShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	unsigned vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	float points[] = {
        // X     Y      Z       U    V      NX     NY     NZ
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
                                           
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f,  1.0f,
                                           
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
                                           
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
                                           
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
                                           
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f
	};

	unsigned vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void*>(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned tex0 = readTexture("/home/stef/Downloads/box.rgb", 512, 512);

	glBindTexture(GL_TEXTURE_2D, tex0);
	int tex0Location = glGetUniformLocation(program, "tex0");
	glUniform1i(tex0Location, 0);

	const float cameraSpeed = 3.f;
	glm::vec3 cameraPos(0.f, 0.f, 3.f);

	float aspect = (float)windowWidth / (float)windowHeight;
	glm::mat4 proj = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.f);

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

		glUseProgram(program);
		glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0);

		glm::mat4 boxModel = glm::rotate(glm::radians(45.f), glm::vec3(1.f, 1.0f, 1.0f));
		glm::mat4 view = rot * glm::translate(-cameraPos);

		setProgramUni(program, proj, "proj");
		setProgramUni(program, view, "view");
		setProgramUni(program, boxModel, "model");
		setProgramUni(program, glm::vec3(0, 2, 0), "lightPos");

		glClearColor(0.6f, 0.6f, 0.6f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// One more time for the light
		glm::mat4 lightModel = glm::translate(glm::vec3(0, 1.5, 0)) * glm::scale(glm::vec3(0.1, 0.1, 0.1));

		glUseProgram(lightProgram);
		setProgramUni(lightProgram, proj, "proj");
		setProgramUni(lightProgram, view, "view");
		setProgramUni(lightProgram, lightModel, "model");
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
	}

	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
