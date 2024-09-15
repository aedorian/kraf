#include "window.h"

void initGlfw() {

	glfwInit(); // init GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // configure GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

}

GLFWwindow* openWindow() {

	// create window
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "kraf", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowPos(window, 310, 150);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD\n";
	}

	
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	// face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	

	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

	return window;
}