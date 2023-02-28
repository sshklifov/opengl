#include <cstdio>
#include <cmath>
#include <cstring>
#include <cassert>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "../include/glad/glad.h"

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

int main() {
	if (!glfwInit()) {
		printf("GLFW failed to init!\n");
		return 1;
	}

	glfwSetErrorCallback(errorCallback);
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Lecture 1", NULL, NULL);
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
		"layout (location = 0) in vec2 aPos;\n"
		"layout (location = 1) in vec2 aTextureCoords;\n"
		"out vec2 st;\n"
		"void main() {\n"
		"	st = aTextureCoords;\n"
		"	gl_Position = vec4(aPos, 0.0, 1.0);\n"
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
		"uniform sampler2D tex;"
		"void main() {\n"
		"	FragColor = texture(tex, st);\n"
		"	//FragColor = vec4(st, 0.0, 1.0);\n"
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
		-0.5f, -0.5f,      // point1
		0.f, 0.f,          // uv1
		0.5f, -0.5f,       // point2
		1.f, 0.f,          // uv2
		0.0f,  0.5f,       // point3
		0.5f, 1.f,          // uv3
	};

	unsigned vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	const int width = 1000;
	const int height = 663;
	size_t numPixels = width * height;
	size_t imageSize = numPixels * 3 * sizeof(unsigned char);
	unsigned char* imageData = static_cast<unsigned char*>(malloc(imageSize));

	FILE * fp = fopen("/home/stef/Downloads/brick.rgb", "r");
	size_t numRead = fread(imageData, 1, imageSize, fp);
	assert(!ferror(fp));
	assert(numRead == numPixels*3);
	assert(!feof(fp));
	int c = fgetc(fp);
	assert(c == EOF);
	assert(feof(fp));

	fclose(fp);

#if 0
	for (size_t i = 0; i < imageSize; i += 1) {
		if (i % 3 == 0) {
			imageData[i] = 255;
		} else {
			imageData[i] = 0;
		}
	}
#endif

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

	while (!glfwWindowShouldClose(window)) {
		glClearColor(1.f, 1.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		glfwPollEvents();

		glUseProgram(program);
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, tex);

		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwSwapBuffers(window);
	}

	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo);

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
