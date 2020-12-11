#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>
#include <windows.h>

#include "Shader.h"

#define CHECK_ERROR() \
{\
    GLenum err = glGetError(); \
    if (err != GL_NO_ERROR) \
    {\
        printf("glGetError returns %d\n", err); \
    }\
}

//enable optimus!
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 1;
    _declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void calculation();

std::string readComputeShaderCode();
GLuint createComputeShader();

GLuint computeShader = 0;

unsigned int width = 1280;
unsigned int height = 720;

#define MIDDLEA -1
#define MIDDLEB 0
#define RANGEA 5.25
#define RANGEB 3

int maxIteration = 100;
int lastIteration = 10000;
double middlea = MIDDLEA;
double middleb = MIDDLEB;
double rangea = RANGEA;
double rangeb = RANGEB;

double zoomd_out = 1;

// create the vertices and the indices
float vertices[] = {
    // positions          // colors           // texture coords
     1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
     1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
};
unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(width, height, "Mandelbrot", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // Set the right window title!
    changeWindowTitle(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set up the render stuff
    unsigned int VBO, VAO, EBO, TEXTURE;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenTextures(1, &TEXTURE);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    CHECK_ERROR();

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TEXTURE);

    // Set basic parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    // Bind the first layer of the texture
    glBindImageTexture(0, TEXTURE, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

    // Create the compute shader program
    computeShader = createComputeShader();

    // Launch the compute shader
    calculation();

    CHECK_ERROR();

    // create the shader program for drawing
    Shader shader("vertexShader.glsl", "fragmentShader.glsl");

    // Render-Loop
    while (!glfwWindowShouldClose(window))
    {
		if (lastIteration != maxIteration)
		{
			calculation();
			lastIteration = maxIteration;
		}

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D, TEXTURE);

        shader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean Up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteTextures(1, &TEXTURE);

    glDeleteProgram(computeShader);
    glDeleteProgram(shader.ID);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
}

void calculation()
{
    //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    // Set compute shader uniforms
    glUseProgram(computeShader);
    glUniform1i(glGetUniformLocation(computeShader, "destTex"), 0);
    glUniform1i(glGetUniformLocation(computeShader, "width"), width);
    glUniform1i(glGetUniformLocation(computeShader, "height"), height);
    glUniform1i(glGetUniformLocation(computeShader, "maxIteration"), maxIteration);
    glUniform1d(glGetUniformLocation(computeShader, "middlea"), middlea);
    glUniform1d(glGetUniformLocation(computeShader, "middleb"), middleb);
    glUniform1d(glGetUniformLocation(computeShader, "rangea"), rangea);
    glUniform1d(glGetUniformLocation(computeShader, "rangeb"), rangeb);

    // Set up time measure and begin measuring
    GLuint myQuery;
    GLuint64 elapsed_time;
    glGenQueries(1, &myQuery);
    glBeginQuery(GL_TIME_ELAPSED, myQuery);

    // Launch the compute shader
    //glDispatchCompute((width + 16 - 1) / 16, (height + 16 - 1) / 16, 1);
    glDispatchCompute((width + 32 - 1) / 32, (height + 32 - 1) / 32, 1);

    // End the timer and print the result
    glEndQuery(GL_TIME_ELAPSED);
    int done = 1;
    while (!done)
        glGetQueryObjectiv(myQuery, GL_QUERY_RESULT_AVAILABLE, &done);
    glGetQueryObjectui64v(myQuery, GL_QUERY_RESULT, &elapsed_time);
    std::cout << elapsed_time << "ns\n";
}

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height)
{
    glViewport(0, 0, _width, _height);
    width = _width;
    height = _height;
    calculation();
}

double xBefore = 0;
double yBefore = 0;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        xBefore = xpos;
        yBefore = ypos;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        //std::cout << "Left mouse button pressed!\n";
        double xDiffernece = xpos - xBefore;
        double yDiffernece = ypos - yBefore;
        middlea = ((middlea * (width / 2) - xDiffernece / zoomd_out) / (width / 2));
        middleb = ((middleb * (height / 2) + yDiffernece / zoomd_out) / (height / 2));
        calculation();
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	rangea += yoffset * 0.25 * rangea;
	rangeb += yoffset * 0.25 * rangeb;
    zoomd_out -= yoffset * 0.5;
    std::cout << zoomd_out << "\n";
    calculation();
}

void changeWindowTitle(GLFWwindow* window)
{
    std::stringstream windowtitle;
    windowtitle << "Mandelbrot " << maxIteration;
    glfwSetWindowTitle(window, windowtitle.str().c_str());
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
        switch(key)
        {
            case GLFW_KEY_I:
            {
                if (mods == GLFW_MOD_SHIFT)
                    maxIteration += maxIteration >= 1000 ? 1000 : maxIteration >= 100 ? 100 : 10;
                else
                    maxIteration -= maxIteration <= 0 ? 0 : maxIteration <= 100 ? 10 : maxIteration <= 1000 ? 100 : 1000;
                changeWindowTitle(window);
                break;
            }
            case GLFW_KEY_R:
            {
                middlea = -1;
                middleb = 0;
                rangea = 5.25;
                rangeb = 3;
                zoomd_out = 1;
            }
            case GLFW_KEY_A:
            {
                middlea -= 0.25 / (RANGEA / rangea);
                calculation();
                break;
            }
            case GLFW_KEY_D:
            {
                middlea += 0.25 / (RANGEA / rangea);
                calculation();
                break;
            }
            case GLFW_KEY_W:
            {
                middleb += 0.25 / (RANGEB / rangeb);
                calculation();
                break;
            }
            case GLFW_KEY_S:
            {
                middleb -= 0.25 / (RANGEB / rangeb);
                calculation();
                break;
            }
            case GLFW_KEY_RIGHT_BRACKET:
            {
                rangea += -1 * 0.25 * rangea;
                rangeb += -1 * 0.25 * rangeb;
                zoomd_out -= -1 * 0.5;
                calculation();
            }
            case GLFW_KEY_SLASH:
            {
                rangea += 1 * 0.25 * rangea;
                rangeb += 1 * 0.25 * rangeb;
                zoomd_out -= 1 * 0.5;
                calculation();
            }
        }
}


std::string readComputeShaderCode()
{
    std::string computeCode;
    std::ifstream cShaderFile;
    cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        cShaderFile.open("computeShader.glsl");
        std::stringstream cShaderStream;
        cShaderStream << cShaderFile.rdbuf();
        cShaderFile.close();
        computeCode = cShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n" << e.what() << std::endl;
    }
    return computeCode;
}

GLuint createComputeShader()
{
    CHECK_ERROR();

    // create the compute shader
    // Read the source code of the shader
    std::string shaderCode = readComputeShaderCode();

    const char* cShaderCode = shaderCode.c_str();

    GLuint computeShader = glCreateProgram();
    GLuint cs = glCreateShader(GL_COMPUTE_SHADER);

    glShaderSource(cs, 1, &cShaderCode, NULL);
    glCompileShader(cs);

    int rvalue;
    glGetShaderiv(cs, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in compiling the compute shader\n");
        GLchar* log = new GLchar[10240];
        GLsizei length;
        glGetShaderInfoLog(cs, 10239, &length, log);
        fprintf(stderr, "Compiler log:\n%s\n", log);
        delete[] log;
        exit(40);
    }
    glAttachShader(computeShader, cs);
    glLinkProgram(computeShader);
    glGetProgramiv(computeShader, GL_LINK_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in linking compute shader program\n");
        GLchar* log = new GLchar[10240];
        GLsizei length;
        glGetProgramInfoLog(computeShader, 10239, &length, log);
        fprintf(stderr, "Linker log:\n%s\n", log);
        delete[] log;
        exit(41);
    }
    return computeShader;
}