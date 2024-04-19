#pragma once

#include <windows.h>

#include <datastructures/datastructures_pch.h>

#include <graphics/gfx_pch.h>


class Renderer {

public:
	Renderer();
	~Renderer();

	bool Initilize(GLFWwindow* window);
	void Cleanup();

	void RenderFrame();
	void PresentFrame();
protected:
	static void HandleResize(GLFWwindow* win, int width, int height);

private:
	// MISC
	void GetRequiredInfo();

	GLFWwindow* window;

};