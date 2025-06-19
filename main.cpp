//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#include <windows.h>
#pragma comment(lib, "Winmm.lib")
#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

//include header flies
#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

//window info
int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

//mouse utils
bool firstMouse{ true };
float lastX{ 400.0f };
float lastY{ 300.0f };
float pitch{ 0.0f };
float yaw{ -90.0f };
const float sensivity{ 0.2f };

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

//matrix declarations
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;

//directional light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// point light parameters
glm::vec3 lightDirPoint;
glm::vec3 lightColorPoint;
glm::vec3 lightPosPoint;

//fogg param
glm::vec3 fogColor;

//location declarations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;

//directional light locations
GLuint lightDirLoc;
GLuint lightColorLoc;

//dir light intensity location
GLint lightnessLoc;

//point light locations
GLint lightDirPointLoc;
GLint lightColorPointLoc;
GLint lightPosPointLoc;

//point light toggle location
GLint turnPointLightLoc;

//fog locations
GLint fogToggleLoc;
GLint fogColorLoc;


gps::Camera myCamera(
	glm::vec3(-4.0f, 6.0f, -27.0f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.01f;

bool pressedKeys[1024];

//directional light angle utils
float angleY = 0.0f;
GLfloat lightAngle;

//3D models instantiation
gps::Model3D turtle;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D scene;
gps::Model3D droplet;

//Shaders instantiation
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader skyBoxShader;

//shadow utils
GLuint shadowMapFBO;
GLuint depthMapTexture;

// toggle depthmap
bool showDepthMap;


// turnLight
float lightness;
float turnPointLight;

//rotate turtle utils
float turtleAngle = 0.0f;
bool isTurtleRotating = false;
float turtleTime;

glm::vec3 turtlePos = glm::vec3(-4.0f, 0.4f, -15.0f);


//fogg toggling
float fogToggle;

//skyboxx declaration
gps::SkyBox mySkyBox;
std::vector<const GLchar*> day_faces, night_faces;

//presentation utils
bool isPresentationActive = false;
float presentationTime = 0.0f;

//render modes util
int renderMode = 0;

//rain drops positions
std::vector<glm::vec3> dropletsPositions;
glm::vec3 rainSpeed = glm::vec3(0.0f, -0.2f, 0.0f);
bool isRaining = false;

//flashh
bool isFlash = false;

//function to check for errors
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
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


//update projection when window is resized
void windowResizeCallback(GLFWwindow* window, int width, int height) {


	glWindowWidth = width;
	glWindowHeight = height;

	glfwGetFramebufferSize(window, &retina_width, &retina_height);

	glViewport(0, 0, retina_width, retina_height);

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);

	myCustomShader.useShaderProgram();
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	std::cout << "Window resized to: " << width << "x" << height << std::endl;
}


//present one island
void animateIslandCamera(float time) {
	float islandRadius = 3.0f;
	glm::vec3 islandOffset(2.0f, 5.0f, 0.0f);

	glm::vec3 islandTarget = glm::vec3(
		islandRadius * sin(time) + islandOffset.x,
		islandOffset.y,
		islandRadius * cos(time) + islandOffset.z
	);

	glm::vec3 cameraOffset = glm::normalize(islandTarget);
	glm::vec3 cameraPos = islandTarget + glm::vec3(cameraOffset.x, 0.0f, cameraOffset.z);

	myCamera.setPosition(cameraPos);
	myCamera.setTarget(islandTarget);
}

//rotate around the scene
void animateOverviewCamera(float time) {
	glm::vec3 overviewTarget(20.0f, 0.0f, 0.0f);

	glm::vec3 orbitPosition = glm::vec3(
		40.0f * sin(time),
		15.0f,
		40.0f * cos(time)
	);

	myCamera.setPosition(orbitPosition);
	myCamera.setTarget(overviewTarget);
}

//start animation
void start_animation() {
	if (!isPresentationActive) return;

	presentationTime += 0.005f;

	float switchTime = 2.0f;

	if (presentationTime < switchTime)
		animateIslandCamera(presentationTime);
	else
		animateOverviewCamera(presentationTime - switchTime);
}


//different render modes switch
void renderModes() {
	if (renderMode == 0) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (renderMode == 1) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (renderMode == 2) {glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glPointSize(5.0f);
	}
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	//close screen
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//show depth map
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	//show presentation
	if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
		isPresentationActive = !isPresentationActive;
		presentationTime = 0.0f;
	}

	//switch between render modes
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		renderMode = (renderMode + 1) % 3;
		renderModes();
	}

	//toggle rotating turtle
	if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		isTurtleRotating = !isTurtleRotating;
	}

	//toggle point light
	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		turnPointLight = abs(turnPointLight - 1.0f);
		myCustomShader.useShaderProgram();
		turnPointLightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "turnPointLight");
		glUniform1fv(turnPointLightLoc, 1, &turnPointLight);
	}

	//toggle fog
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		fogToggle = abs(fogToggle - 0.0125f);
		myCustomShader.useShaderProgram();
		fogToggleLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogToggle");
		glUniform1fv(fogToggleLoc, 1, &fogToggle);
	}

	//toggle rain
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		isRaining = !isRaining;
	}

	//flash scene
	if (key == GLFW_KEY_L && action == GLFW_PRESS) {
		isFlash = true;
	}

	//increase directional light intensity
	if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT) && lightness < 1.0f) {
		if (fabs(lightness - 0.5f) < 1e-6) {
			mySkyBox.Load(day_faces);
			mySkyBox.Draw(skyBoxShader, myCamera.getViewMatrix(), projection);
		}
		lightness += 0.05f;
		myCustomShader.useShaderProgram();
		lightnessLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightness");
		glUniform1fv(lightnessLoc, 1, &lightness);
	}

	//decrease directional light intensity
	if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT) && lightness > 0.0f) {
		if (fabs(lightness - 0.5f) < 1e-6) {
			mySkyBox.Load(night_faces);
			mySkyBox.Draw(skyBoxShader, myCamera.getViewMatrix(), projection);
		}
		lightness -= 0.05f;
		myCustomShader.useShaderProgram();
		lightnessLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightness");
		glUniform1fv(lightnessLoc, 1, &lightness);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xPos, double yPos) {

	if (firstMouse) {
		lastX = static_cast<float>(xPos);
		lastY = static_cast<float>(yPos);
		firstMouse = false;
	}

	float xOffset = static_cast<float>(xPos) - lastX;
	float yOffset = lastY - static_cast<float>(yPos);

	lastX = static_cast<float>(xPos);
	lastY = static_cast<float>(yPos);

	xOffset *= sensivity;
	yOffset *= sensivity;

	yaw += xOffset;
	pitch += yOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

void processMovement()
{	
	//rotate directional light left
	if (pressedKeys[GLFW_KEY_LEFT]) {
		lightAngle -= 1.0f;
	}

	//rotate directional light right
	if (pressedKeys[GLFW_KEY_RIGHT]) {
		lightAngle += 1.0f;
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
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}


void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

//initialize objects
void initObjects() {
	turtle.LoadModel("objects/turtle/turtle.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	scene.LoadModel("objects/scene/project.obj");
	droplet.LoadModel("objects/rain/drop.obj");
}

//initialize shaders
void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();
	skyBoxShader.loadShader("shaders/skyBoxShader.vert", "shaders/skyBoxShader.frag");
	skyBoxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	//set the model matrix
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//set the view matrix
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//set the normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	//set the projection matrix
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the POINT light direction
	lightDirPoint = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirPointLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirPoint");
	glUniform3fv(lightDirPointLoc, 1, glm::value_ptr(lightDirPoint));

	//set POINT light color
	lightColorPoint = glm::vec3(1.0f, 1.0f, 0.1f);
	lightColorPointLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColorPoint");
	glUniform3fv(lightColorPointLoc, 1, glm::value_ptr(lightColorPoint));

	//set the POINT light position
	lightPosPoint = glm::vec3(13.0f, 10.0f, 27.75f);
	lightPosPointLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPosPoint");
	glUniform3fv(lightPosPointLoc, 1, glm::value_ptr(lightPosPoint));

	//set light on/off
	turnPointLight = 0.0f;
	turnPointLightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "turnPointLight");
	glUniform1fv(turnPointLightLoc, 1, &turnPointLight);

	//set the directional light direction
	lightDir = glm::vec3(0.0f, 25.0f, 60.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set the directional light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//set the directional light intensity
	lightness = 1.0f;
	lightnessLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightness");
	glUniform1fv(lightnessLoc, 1, &lightness);

	//set fog on/off
	fogToggle = 0.0f;
	fogToggleLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogToggle");
	glUniform1fv(fogToggleLoc, 1, &fogToggle);

	//set fog color
	fogColor = glm::vec3(0.75f, 0.75f, 0.75f);
	fogColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogColor");
	glUniform3fv(fogColorLoc, 1, glm::value_ptr(fogColor));


	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

//initialize FBO for shadow mapping
void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//initialize rain
void initDrops(int number) {
	for (int i = 0; i < number; i++) {
		dropletsPositions.push_back
		(glm::vec3(
			static_cast<float>(rand()) / RAND_MAX * 160.0f - 80.0f,
			static_cast<float>(rand()) / RAND_MAX * 40.0f + 20.0f,
			static_cast<float>(rand()) / RAND_MAX * 160.0f - 80.0f));
	}
}

//update droplets position and reinitialize them if they go below map
void reInitDrops() {
	if (isRaining) {
		for (glm::vec3& rainDropPos : dropletsPositions) {
			rainDropPos += rainSpeed;
			if (rainDropPos.y < -20.0f)
				rainDropPos = glm::vec3(
					static_cast<float>(rand()) / RAND_MAX * 160.0f - 80.0f,
					static_cast<float>(rand()) / RAND_MAX * 40.0f + 20.0f,
					static_cast<float>(rand()) / RAND_MAX * 160.0f - 80.0f
				);
		}
	}
}

//flash scene and play thunder sound
void initFlash() {
	if (isFlash) {
		myCustomShader.useShaderProgram();
		glm::vec3 lightColor = glm::vec3(10.0f, 10.0f, 10.0f);
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
		PlaySound(TEXT("objects/thunder/thunder.wav"), NULL, SND_FILENAME | SND_ASYNC);
		isFlash = false;
	}
}

//go back to standard light
void destroyFlash() {
	if (!isFlash) {
		myCustomShader.useShaderProgram();
		glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
		glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	}
}

//compute the transformation matrix for light space
glm::mat4 computeLightSpaceTrMatrix() {
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightView = glm::lookAt(glm::vec3(lightRotation * glm::vec4(lightDir, 1.0f)), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightProjection = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, 0.1f, 160.0f);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

//draw the rotating turtle
void drawRotatingTurtle(gps::Shader shader) {

	shader.useShaderProgram();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, turtlePos); //translate turtle back into the original place

	//increase the angle of the rotation as animation time passes
	if (isTurtleRotating) {
		turtleAngle += 0.35f * turtleTime;
	}

	model = glm::rotate(model, glm::radians(turtleAngle), glm::vec3(0.0f, 1.0f, 0.0f)); //rotate turtle with the given angle
	model = glm::translate(model, -turtlePos); //translate turtle into the origin

	modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(shader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	turtle.Draw(shader);// draw the turtle
}

//draw rain drops
void drawRain(gps::Shader shader) {
	shader.useShaderProgram();
	if (isRaining) {
		for (glm::vec3 dropPos : dropletsPositions) {
			glm::mat4 model = glm::translate(glm::mat4(1.0f), dropPos);
			modelLoc = glGetUniformLocation(shader.shaderProgram, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			droplet.Draw(shader);
		}
	}
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	scene.Draw(shader);
}

void renderScene() {

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//depth map rendering
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else { // render the scene with shadows

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false); //draw scene
		drawRotatingTurtle(myCustomShader); //draw the turtle
		drawRain(myCustomShader);//draw rain drops

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 100.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader); //draw the light cube

		mySkyBox.Draw(skyBoxShader, view, projection); //draw the skybox
	}
}

void initSkyBox() {
		
		//prepare the day skybox
		day_faces.push_back("skybox/bluecloud_rt.jpg");
		day_faces.push_back("skybox/bluecloud_lf.jpg");
		day_faces.push_back("skybox/bluecloud_up.jpg");
		day_faces.push_back("skybox/bluecloud_dn.jpg");
		day_faces.push_back("skybox/bluecloud_bk.jpg");
		day_faces.push_back("skybox/bluecloud_ft.jpg");

		//prepare the night skybox
		night_faces.push_back("skybox/browncloud_rt.jpg");
		night_faces.push_back("skybox/browncloud_lf.jpg");
		night_faces.push_back("skybox/browncloud_up.jpg");
		night_faces.push_back("skybox/browncloud_dn.jpg");
		night_faces.push_back("skybox/browncloud_bk.jpg");
		night_faces.push_back("skybox/browncloud_ft.jpg");
	
	//load day skybox as default
	mySkyBox.Load(day_faces);
}

//cleanup util
void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	glfwTerminate();
}


//main function
int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initSkyBox();
	initDrops(20000);
	initFBO();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		turtleTime += 0.005f; //increase turtle time to rotate it smoothly
		processMovement();
		reInitDrops();
		renderScene();
		destroyFlash();
		initFlash();
		start_animation();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
