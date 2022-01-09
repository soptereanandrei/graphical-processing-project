//
//  main.cpp
//  OpenGL_Shader_Example_step1
//
//  Created by CGIS on 02/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include <vector>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp>//glm extension for inverse
#include "glm/gtc/type_ptr.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

glm::mat4 model;
GLint modelLoc;

glm::mat4 view;
GLint viewLoc;

glm::mat4 projection;
GLint projectionLoc;

glm::vec3 camPosition = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 camTarget = glm::vec3(0.0f, 2.0f, -10.0f);

gps::Camera myCamera(glm::vec3(0.0f, 5.0f, 15.0f), glm::vec3(0.0f, 2.0f, -10.0f), glm::vec3(0, 1, 0));
float cameraSpeed = 0.01f;

bool pressedKeys[1024];
float angle = 0.0f;

gps::Model3D houseModel;
gps::Model3D pewterModel;
gps::Model3D sideboardModel;
gps::Model3D groundModel;
gps::Model3D tableModel;
gps::Model3D door;
gps::Model3D trashCanModel;
gps::Model3D lightModel;

bool showDepthMap = false;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

double xPosLast, yPosLast;
double mouseSpeed = 0.01f;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (xpos != xPosLast || ypos != yPosLast)
	{
		double deltaX = xpos - xPosLast;
		double deltaY = ypos - yPosLast;

		xPosLast = xpos;
		yPosLast = ypos;

		float pitch = -deltaY * mouseSpeed;
		float yaw = -deltaX * mouseSpeed;
		myCamera.rotate(pitch, yaw);

		std::cout << "deltaX=" << deltaX << " deltaY=" << deltaY << "\n";
		//std::cout << pitch << " " << yaw << "\n";
	}
}

void processMovement()
{

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 0.0002;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 0.0002;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_UP]) {
		camPosition.z -= 0.1;
		camTarget.z -= 0.1;
	}
	if (pressedKeys[GLFW_KEY_DOWN]) {
		camPosition.z += 0.1;
		camTarget.z += 0.1;
	}
}



GLuint ReadTextureFromFile(const char* file_name) {
	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = stbi_load(file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(
			stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
		);
	}

	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_SRGB, //GL_SRGB,//GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
	);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

//GLuint verticesVBO;
//GLuint verticesEBO;
//GLuint objectVAO;
//GLint texture;

glm::mat3 normalMatrix;
glm::mat4 lightSpaceMatrix;

gps::Shader lightingShader;
//glm::vec3 lightDir(0.0f, 1.0f, 1.0f);
glm::vec3 lightDir(-4.0f, 4.0f, -1.0f);
GLint lightDirLoc;
glm::vec3 lightColor(0.3f, 0.3f, 0.3f);
GLint lightColorLoc;
glm::vec3 viewPosEye;

const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

GLuint depthMapFBO;
GLuint depthMap;
gps::Shader depthShader;
gps::Shader screenQuadShader;
gps::Model3D screenQuad;

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);

	glCheckError();
}

void initObjects()
{
	houseModel.LoadModel("objects\\House\\house.obj", "objects\\House\\");
	groundModel.LoadModel("objects\\Ground\\ground.obj", "objects\\Ground\\");
	pewterModel.LoadModel("objects\\mirabel_98_pewter\\mirabel_98_pewter.obj", "objects\\mirabel_98_pewter\\");
	sideboardModel.LoadModel("objects\\Sideboard\\sideboard.obj", "objects\\Sideboard\\");
	tableModel.LoadModel("objects\\Table\\table.obj", "objects\\Table\\");
	door.LoadModel("objects\\Door\\door.obj", "objects\\Door\\");
	trashCanModel.LoadModel("objects\\TrashCan\\TrashCan.obj", "objects\\TrashCan\\");
	lightModel.LoadModel("objects\\light\\light.obj", "objects\\light\\");
	screenQuad.LoadModel("objects/quad/quad.obj");

	glCheckError();
}

void initShaders()
{
	lightingShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightingShader.useShaderProgram();
	depthShader.loadShader("shaders/depthShader.vert", "shaders/depthShader.frag");
	depthShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();

	glCheckError();
}

void initUniforms()
{
	lightingShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(lightingShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glCheckError();

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(lightingShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glCheckError();

	lightDirLoc = glGetUniformLocation(lightingShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * model)) * lightDir));
	glCheckError();

	lightColorLoc = glGetUniformLocation(lightingShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	glCheckError();
}

void initFBO()
{
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCheckError();
}

void computeLightSpaceTrMatrix()
{
	//render scene to compute depth map
	/*float near_plane = 0.1f, far_plane = 5.0f;
	glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.f, 1.0f, near_plane, far_plane);*/
	float near_plane = 1.0f, far_plane = 7.5f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	lightSpaceMatrix = lightProjection * lightView;

	glCheckError();
}

void renderDepthMap()
{
	//render for depth map
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	depthShader.useShaderProgram();

	computeLightSpaceTrMatrix();
	GLuint lightSpaceMatrixLoc = glGetUniformLocation(depthShader.shaderProgram, "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	houseModel.Draw(depthShader);
	pewterModel.Draw(depthShader);
	sideboardModel.Draw(depthShader);
	groundModel.Draw(depthShader);
	tableModel.Draw(depthShader);
	door.Draw(depthShader);
	trashCanModel.Draw(depthShader);
	lightModel.Draw(depthShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glCheckError();
}

void renderObjects()
{
	glViewport(0, 0, retina_width, retina_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lightingShader.useShaderProgram();
	
	//initialize the model matrix
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(lightingShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//send matrix data to shader
	view = myCamera.getViewMatrix();
	GLint viewLoc = glGetUniformLocation(lightingShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//send projection matrix to shader
	GLint projLoc = glGetUniformLocation(lightingShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	GLint normalMatrixLoc = glGetUniformLocation(lightingShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	GLint lightSpaceMatrixLoc = glGetUniformLocation(lightingShader.shaderProgram, "lightSpaceMatrix");
	glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

	GLint lightDirLoc = glGetUniformLocation(lightingShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view)) * lightDir));
	//glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//compute view position in eye coordinates
	viewPosEye = view * model * glm::vec4(myCamera.getCameraPosition(), 1.0f);
	GLint viewPosEyeLoc = glGetUniformLocation(lightingShader.shaderProgram, "viewPosEye");
	glUniform3fv(viewPosEyeLoc, 1, glm::value_ptr(viewPosEye));

	//bind the shadow map
	GLint shadowMapLoc = glGetUniformLocation(lightingShader.shaderProgram, "shadowMap");
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(shadowMapLoc, 3);
	

	houseModel.Draw(lightingShader);
	pewterModel.Draw(lightingShader);
	sideboardModel.Draw(lightingShader);
	groundModel.Draw(lightingShader);
	tableModel.Draw(lightingShader);
	door.Draw(lightingShader);
	trashCanModel.Draw(lightingShader);
	lightModel.Draw(lightingShader);

	glCheckError();
}

void showShadowMap()
{
	glViewport(0, 0, retina_width, retina_height);

	glClear(GL_COLOR_BUFFER_BIT);

	screenQuadShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

	glDisable(GL_DEPTH_TEST);
	screenQuad.Draw(screenQuadShader);
	glEnable(GL_DEPTH_TEST);

	glCheckError();
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.8, 0.8, 0.8, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	processMovement();

	renderDepthMap();
	if (showDepthMap)
		showShadowMap();
	else
		renderObjects();
}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}

//gps::Shader debugShader;
//void debugShadowMap()
//{
//	glViewport(0, 0, retina_width, retina_height);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	debugShader.useShaderProgram();
//
//	//initialize the model matrix
//	model = glm::mat4(1.0f);
//	modelLoc = glGetUniformLocation(debugShader.shaderProgram, "model");
//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//
//	//send matrix data to shader
//	view = myCamera.getViewMatrix();
//	GLint viewLoc = glGetUniformLocation(debugShader.shaderProgram, "view");
//	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//
//	//send projection matrix to shader
//	GLint projLoc = glGetUniformLocation(debugShader.shaderProgram, "projection");
//	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//
//	glActiveTexture(GL_TEXTURE0);
//	glUniform1i(glGetUniformLocation(debugShader.shaderProgram, "depthMap"), 0);
//	glBindTexture(GL_TEXTURE_2D, depthMap);
//
//	unsigned int quadVAO = 0;
//	unsigned int quadVBO;
//
//	float quadVertices[] = {
//		// positions        // texture Coords
//		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
//		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
//		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
//		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
//	};
//	// setup plane VAO
//	glGenVertexArrays(1, &quadVAO);
//	glGenBuffers(1, &quadVBO);
//	glBindVertexArray(quadVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
//	glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(1);
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
//
//	glBindVertexArray(quadVAO);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//	glBindVertexArray(0);
//}