#include <cstdio>

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

    float points[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };

    unsigned vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(vbo, sizeof(points), points, GL_STATIC_DRAW);

    const char* vertexShaderSource =
        "#version 330\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
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
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = vec4(0.6, 0.2, 1.0, 1.0);\n"
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

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.6f, 0.2f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        glfwPollEvents();
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
    }

    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
