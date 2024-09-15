#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

#include "window.h"
#include "block.h"
#include "chunk.h"
#include "world.h"
#include "texture.h"
#include "renderer.h"
#include "raycast.h"

#include "shader.h"
#include "camera.h"

#define MOUSE_LEFT -1
#define MOUSE_RIGHT 1

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
int hasPlayerMovedXZ(GLFWwindow *window);
int getMouseButton(GLFWwindow *window);



static GLFWwindow* window;

// camera settings
Camera camera(glm::vec3(0.0f, 35.0f, 0.0f));
float lastX = WIN_WIDTH / 2.0f;
float lastY = WIN_HEIGHT / 2.0f;
bool firstMouse = true;
bool waitReleaseLeft = false, waitReleaseRight = false; // wait for mouse button to release

// for frame time logic
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

/* GAME DATA */
unsigned int blocksTexture;

const int inventorySize = 9;
BlockType inventory[inventorySize] = { BlockType::PLANKS, BlockType::LOG, BlockType::STONE,
									   BlockType::DIRT, BlockType::GRASS, BlockType::COBBLE,
								       BlockType::SAND, BlockType::LEAVES, BlockType::AIR } ; // would be better in a Player class
int inventoryIndex = 0; // currently selected block in inventory



void mainLoop() {

	// compile shaders
	Shader chunkShader("shaders/chunk_v.vert", "shaders/chunk_f.frag"); // for rendering a chunk
	Shader blockShader("shaders/block_v.vert", "shaders/block_f.frag"); // for rendering a single block (inventory block, sun, moon)
	// get texture atlas
	blocksTexture = createTexture("res/blocks_atlas.png");
	chunkShader.use();
	chunkShader.setInt("textures", 0);
	blockShader.use();
	blockShader.setInt("textures", 0);

	// create world
	World world;
	world.init();

	// initialize single block data
	BlockModel blockModel;
	blockModel.init();

	// create raycast helper
	Raycast raycast;
	raycast.init();




	// main gmae loop

	while (!glfwWindowShouldClose(window))
	{

		// frame time logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		float fps = 1.0f / deltaTime;

		// set window title to show fps
		std::stringstream ss;
		ss << "kraf | " << fps << " FPS";
		glfwSetWindowTitle(window, ss.str().c_str());


		processInput(window);


		// all the chunk stuff is happening here
		// only update the chunk list if the player has moved
		if (firstMouse) {
			world.worldUpdate(&camera, deltaTime, 1);
		}
		else {
			world.worldUpdate(&camera, deltaTime, hasPlayerMovedXZ(window));
		}

		// rendering

		glm::vec3 skyColor = world.calculateSkyColor(world.time);
		glClearColor(skyColor.x, skyColor.y, skyColor.z , 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		chunkShader.use();
		prepareShaderMatrices(&chunkShader, &camera);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, blocksTexture);

		// blockShader.use();
		prepareShaderMatrices(&blockShader, &camera);

		world.renderWorld(&chunkShader, &blockShader, &blockModel, &camera);

		// raycasting and block breaking/placing
		prepareShaderMatrices(raycast.getShader(), &camera);
		if (raycast.raycast(window, &world, &camera)) {
			// a block was hit
			int mouseAction = getMouseButton(window);
			if (mouseAction == MOUSE_LEFT) {
				raycast.breakBlock();
			}
			if (mouseAction == MOUSE_RIGHT) {
				raycast.placeBlock(inventory[inventoryIndex]);

			}
		}



		// render inventory block in the corner of the screen
		prepareShaderMatrices(&blockShader, &camera);
		blockModel.renderBlock(&blockShader,
			camera.Position + camera.Right * 1.25f + camera.Front - camera.Up * 0.75f,
			glm::vec3(camera.Pitch, 0.0f, 0.0f),
			glm::vec3(0.5f, 0.5f, 0.5),
			inventory[inventoryIndex]);
		


		// swap buffers and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


}

int main() {

	initGlfw();
	window = openWindow();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);


	mainLoop();


	glfwTerminate();
	return 0;
}

// returns 0 if nothing pressed, -1 for left, 1 for right
int getMouseButton(GLFWwindow *window) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE && waitReleaseLeft) {
		waitReleaseLeft = false;
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !waitReleaseLeft) {
		waitReleaseLeft = true;
		return MOUSE_LEFT;
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && waitReleaseRight) {
		waitReleaseRight = false;
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !waitReleaseRight) {
		waitReleaseRight = true;
		return MOUSE_RIGHT;
	}
	return 0;
}

int hasPlayerMovedXZ(GLFWwindow *window) {
	return (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
		|| glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS
		|| glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS
		|| glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.MovementSpeed = 20.0f;
	else camera.MovementSpeed = 10.0f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// change inventory selected block

	int offset = int(yoffset);
	
	inventoryIndex += offset;

	if (inventoryIndex >= inventorySize) {
		inventoryIndex = 0;
	}
	if (inventoryIndex < 0) {
		inventoryIndex = inventorySize - 1;
	}
}