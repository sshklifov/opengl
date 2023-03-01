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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,  GL_RGB, GL_UNSIGNED_BYTE, imageData); // TODO try other internal format
	glGenerateMipmap(GL_TEXTURE_2D);
	free(imageData);

	return tex;
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
	glfwSetKeyCallback(window, keyCallback);

	glfwMakeContextCurrent(window);
	if (!gladLoadGL()) {
		printf("Glad failed to init!\n");
		return 1;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, resizeCallback);

	// rad = deg / 180 * pi
	// deg = rad / pi * 180

	const char* vertexShaderSource =
		"#version 330\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec2 aTextureCoords;\n"
		"uniform mat4 proj;\n"
		//"uniform mat4 model;\n"
		"out vec2 st;\n"
		"void main() {\n"
		"	gl_Position = proj * vec4(aPos.x, aPos.y, aPos.z - 3.0, 1.0);\n"
		//"	gl_Position = proj * vec4(aPos, 1.0);\n"
		"	st = aTextureCoords;\n"
		"}\n";
	unsigned vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int success = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		printf("Problem with compilation of vertex shader!\n");
		char info[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, info);
		printf("Shader info log: %s\n", info);
	}

	const char* fragmentShaderSource =
		"#version 330\n"
		"in vec2 st;\n"
		"out vec4 FragColor;\n"
		"uniform sampler2D tex0;"
		"uniform sampler2D tex1;"
		"void main() {\n"
		"	FragColor = texture(tex1, st);\n"
		"}\n";
	unsigned fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	success = 1;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		printf("Problem with compilation of fragment shader!\n");
		char info[512];
		glGetShaderInfoLog(fragmentShader, 512, NULL, info);
		printf("Shader info log: %s\n", info);
	}

	unsigned program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		printf("Problem with linking of shading program!\n");
	}
	glUseProgram(program);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	unsigned vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	float points[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	unsigned vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned tex0 = readTexture("/home/stef/Downloads/brick.rgb", 1000, 663);

	glBindTexture(GL_TEXTURE_2D, tex0);
	int tex0Location = glGetUniformLocation(program, "tex0");
	glUniform1i(tex0Location, 0);

	unsigned tex1 = readTexture("/home/stef/Downloads/tree.rgb", 3000, 2000);

	glBindTexture(GL_TEXTURE_2D, tex1);
	int tex1Location = glGetUniformLocation(program, "tex1");
	glUniform1i(tex1Location, 1);

	float aspect = (float)windowWidth / (float)windowHeight;
	glm::mat4 proj = glm::perspective(glm::radians(45.f), aspect, 0.01f, 100.f);

	int projLocation = glGetUniformLocation(program, "proj");
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, glm::value_ptr(proj));

	while (!glfwWindowShouldClose(window)) {
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwPollEvents();

		glUseProgram(program);
		glBindVertexArray(vao);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex1);

		glDrawArrays(GL_TRIANGLES, 0, 36);
		glfwSwapBuffers(window);
	}

	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
