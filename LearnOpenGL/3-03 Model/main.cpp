﻿#include <iostream>
#include <cmath>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL/SOIL.h>
// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

// Camera
Camera  camera(glm::vec3(5.0f, 15.0f, 15.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];

// Light attributes
glm::vec3 lightPos(0.0f, 10.0f, 8.0f);

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

							// The MAIN function, from here we start the application and run the game loop
int main()
{
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);


	// Build and compile our shader program
	Shader lightingShader("Resources/Shaders/diffuse.vs", "Resources/Shaders/diffuse.frag");
	Shader sampleShader("Resources/Shaders/sample.vs", "Resources/Shaders/sample.frag");

	// Load model
	Model nanosuit("Resources/Models/nanosuit/nanosuit.obj");
	Model ball("Resources/Models/alien_ball_bearing.stl");


	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		do_movement();

		// Clear the colorbuffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Use cooresponding shader when setting uniforms/drawing objects
		

		// Create camera transformations
		glm::mat4 view;
		view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

		// 绘制人物模型


		lightingShader.Use();

		// Get the uniform locations
		GLint modelLoc = lightingShader.GetUniformLocation("model");
		GLint viewLoc = lightingShader.GetUniformLocation("view");
		GLint projLoc = lightingShader.GetUniformLocation("projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));


		// light
		lightingShader.SetUniformValue("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
		lightingShader.SetUniformValue("light.position", camera.Position.x, camera.Position.y, camera.Position.z);
		lightingShader.SetUniformValue("light.direction", camera.Front.x, camera.Front.y, camera.Front.z);
		lightingShader.SetUniformValue("light.cutOff", glm::cos(glm::radians(12.5f)));
		lightingShader.SetUniformValue("light.outerCutOff", glm::cos(glm::radians(17.5f)));

		lightingShader.SetUniformValue("light.ambient", 0.1f, 0.1f, 0.1f);
		// We set the diffuse intensity a bit higher; note that the right lighting conditions differ with each lighting method and environment.
		// Each environment and lighting type requires some tweaking of these variables to get the best out of your environment.
		lightingShader.SetUniformValue("light.diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader.SetUniformValue("light.specular", 1.0f, 1.0f, 1.0f);
		lightingShader.SetUniformValue("light.constant", 1.0f);
		lightingShader.SetUniformValue("light.linear", 0.09);
		lightingShader.SetUniformValue("light.quadratic", 0.032);



		glm::mat4 model = glm::mat4();


	    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		nanosuit.Draw(lightingShader);

		//绘制灯模型
		sampleShader.Use();

		// Get the uniform locations
		modelLoc = glGetUniformLocation(sampleShader.Program, "model");
		viewLoc = glGetUniformLocation(sampleShader.Program, "view");
		projLoc = glGetUniformLocation(sampleShader.Program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Draw 10 containers with the same VAO and VBO information; only their world space coordinates differ
		model = glm::mat4();

		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.01, 0.01, 0.01));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		ball.Draw(sampleShader);



		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}