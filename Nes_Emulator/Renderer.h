#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "ErrorHandle.h"
class Renderer
{
public:
	Renderer();
	void StartPPU();

private:
	GLFWwindow* window;
};

