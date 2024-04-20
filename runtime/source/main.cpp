#include <iostream>
#include <unordered_set>
#include<algorithm>

#include <GLFW/glfw3.h>

#include <graphics/Rendering/renderer.h>
#include <debug/Console.h>

namespace glfw
{
	GLFWwindow* window;
}

const int HEIGHT = 720;
const int WIDTH = (int)(HEIGHT * (16.f / 9.f));

void InitilizeWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	glfw::window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

Renderer renderer;


#define ENABLE_AUTOMATED_TESTING 1
#define ENABLE_VISUAL_TESTING 1

#if ENABLE_AUTOMATED_TESTING
#include "UnitTests.hpp"
#endif 


int main() {
#if ENABLE_AUTOMATED_TESTING 
	_
#endif

#if ENABLE_VISUAL_TESTING

	Console::Log("Process Started.");
	Console::Log("Starting Test");

	Console::Log("Initillizing Window ", "Width: ", WIDTH, " Height: ", HEIGHT);
	InitilizeWindow();
	Console::Success("Window Initilized Successfully");


	Console::Log("Initillizing Renderer ");
	if (!renderer.Initilize(glfw::window))
		return 1;

	Console::Success("Renderer Initilized Successfully");
	Console::Log("Starting Update Loop");

	while (!glfwWindowShouldClose(glfw::window)) {
		//Console::Info("Updating");

		renderer.RenderFrame();
		renderer.PresentFrame();
		glfwPollEvents();
	}
	Console::Log("Application Closed, Performing Cleanup.");
	renderer.Cleanup();

	Console::Log("Cleanup Finished Closing Process.");
#endif

	return 0;
}

