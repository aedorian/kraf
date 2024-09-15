#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

void initGlfw();

GLFWwindow* openWindow();

#endif /* _WINDOW_H_ */