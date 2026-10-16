#include <glad/gl.h>  /* Using GLAD (download him -- version 4.6 and profile: core && common)*/
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <fstream>
#define STB_TRUETYPE_IMPLEMENTATION
#include "lib/stb_truetype.h"

/* Load shader from file.
 * Job: load shader from file in std::string.
*/
std::string loadShaderFromFile(const std::string& filepath) {
	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Error, failed while open file shader: " << filepath << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

unsigned int compileShader(unsigned int type, const std::string& source) {
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id,1,&src,nullptr);
	glCompileShader(id);

	int success;
	char infoLog[512];
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(id,512,NULL,infoLog);
		std::cerr << "Failed while compiling shader (" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "):\n" << infoLog << std::endl;
	}
	return id;	
}

/* Main of A+ Engine. 
 * Job: Create window.
 * Using OpenGL 4.6.
*/
int main() {
	/* Init GLFW. */
	if (!glfwInit()) {
		std::cerr << "An error accured. Failed to initialize GLFW" << std::endl;
		//glfwTerminate();
		return -1;
	}
	/* Set OpenGL 4.6 version. */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create OpenGL window. */
	GLFWwindow* window = glfwCreateWindow(1280, 720, "A+ Engine", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	int version = gladLoadGL(glfwGetProcAddress);
	/* let's init ptr to OpenGL 4.6 functions */
	if (version == 0) {
		std::cerr << "Failure: failed while GLAD is intializing." << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	std::cout << "The OpenGL context is ok." << std::endl;
	std::cout << "Version " << GLAD_VERSION_MAJOR(version) << "." << GLAD_VERSION_MINOR(version) << std::endl;
	FILE* fontFile = fopen("fonts/gordin-regular.ttf", "rb");
	if (!fontFile) {
		std::cerr << "Error, file 'fonts/gordin-regular.ttf' not found" << std::endl;
		return -1;
	}
	fseek(fontFile, 0, SEEK_END);
	long fileSize = ftell(fontFile);
	fseek(fontFile,0,SEEK_SET);
	unsigned char* fontBuffer = (unsigned char*)malloc(fileSize);
	fread(fontBuffer, 1, fileSize, fontFile);
	fclose(fontFile);
	stbtt_fontinfo fontInfo;
	if (!stbtt_InitFont(&fontInfo, fontBuffer, 0)) {
		std::cerr << "Error while initializing shader" << std::endl;
		return -1;
	}
	float scale = stbtt_ScaleForPixelHeight(&fontInfo, 64.0f);
	int x0, y0, x1, y1;
	stbtt_GetCodepointBitmapBox(&fontInfo, 'A', scale, scale, &x0, &y0, &x1, &y1);
	int width = x1 - x0;
	int height = y1 - y0;
	unsigned char* bitmap = (unsigned char*)malloc(width * height);
	stbtt_MakeCodepointBitmap(&fontInfo, bitmap, width, height, width, scale, scale, 'A');
	unsigned int textTexture;
	glGenTextures(1, &textTexture);
	glBindTexture(GL_TEXTURE_2D, textTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

	free(bitmap);
	free(fontBuffer);
	float aspectRatio = (float)width / (float)height;
	float quadHeight = 0.5f; // Высота квада в NDC (от -0.5 до 0.5 = 1.0)
	float quadWidth = quadHeight * aspectRatio; // Ширина подстраивается под пропорции

	float quadVertices[] = {
		// pos                    // texture
		-quadWidth,  quadHeight, 0.0f,  0.0f, 0.0f, // Верхний левый
		-quadWidth, -quadHeight, 0.0f,  0.0f, 1.0f, // Нижний левый
		 quadWidth, -quadHeight, 0.0f,  1.0f, 1.0f, // Нижний правый
		 quadWidth,  quadHeight, 0.0f,  1.0f, 0.0f  // Верхний правый
	};
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	// atr0: pos (3 float)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// atr1: texture (2 float)
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
	std::string vertSource = loadShaderFromFile("assets/shaders/text.vert");
	std::string fragSource = loadShaderFromFile("assets/shaders/text.frag");
	if (vertSource.empty() || fragSource.empty()) {
		std::cerr << "Fatal error: shaders is not loaded!" << std::endl;
		return -1;
	}
	unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertSource);
	unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragSource);
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/* the main cycle of engine. 
     * esc -- exit from engine
	*/
	while (!glfwWindowShouldClose(window)) {
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		int textColorLoc = glGetUniformLocation(shaderProgram, "textColor");
		glUniform3f(textColorLoc, 1.0f, 1.0f, 1.0f);
		glBindVertexArray(VAO);
		glBindTexture(GL_TEXTURE_2D, textTexture);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
