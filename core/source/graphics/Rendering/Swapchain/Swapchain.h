#pragma once

#include <graphics/gfx_pch.h>





class Swapchain {

public:
	Swapchain(struct GLFWwindow* window, VkSurfaceKHR& surface, VulkanAPI::QueueFamily queueFamily, Resolution resolution, uint32_t images = 3);
	~Swapchain();


	VkSwapchainKHR Get();
	VkImage GetImage(int idx);
	
	uint32_t ImageCount();
	TReturn AquireNextImage(VkSemaphore& imageAvailableSemaphore, uint32_t& currentFrameIndex);

private:
	TReturn Create(VkSurfaceKHR& surface);

	VulkanAPI::QueueFamily queueFamily;
	VkSwapchainKHR swapchain;

	Resolution resolution;

	std::vector<VkImage> swapchainImages;

	struct GLFWwindow* window;
};