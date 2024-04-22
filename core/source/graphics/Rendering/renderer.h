#pragma once

#include <defs.h>
#include <datastructures/datastructures_pch.h>

struct GLFWwindow;
class CommandManager;
class Framebuffer;
class Swapchain;
class RenderPipeline;

class Renderer {

public:
	Renderer();
	~Renderer();

	TReturn Initilize(GLFWwindow* window);
	
	void Cleanup();

	void RenderFrame();
	void PresentFrame();
protected:
	static void HandleResize(GLFWwindow* win, int width, int height);

private:

	GLFWwindow* window;



	CommandManager* commandManager;
	Framebuffer* framebuffer;
	Swapchain* swapchain;
	
	std::unordered_map<std::string, RenderPipeline*> renderPipelines;

};

