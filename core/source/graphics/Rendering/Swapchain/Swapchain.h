#pragma once

#include <graphics/gfx_pch.h>

struct SwapchainSupportDetails {
	VkSurfaceCapabilitiesKHR capabillities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};




class Swapchain {

public:
	Swapchain(VkDevice device, VkPhysicalDevice physicalDevice, struct GLFWwindow* window, VkSurfaceKHR surface, VulkanAPI::QueueFamily queueFamily, Resolution resolution, uint32_t images = 3);
	~Swapchain();

	void Create();

	//void Resize(Resolution resolution);

	VkSwapchainKHR Get();
	VkImage GetImage(int idx);

private:
	SwapchainSupportDetails GetSupportDetails();

	VkSurfaceFormatKHR SelectFormat(VkFormat format, VkColorSpaceKHR colorSpace);
	VkPresentModeKHR SelectPresentMode(VkPresentModeKHR presentMode);
	VkExtent2D SelectExtent(const VkSurfaceCapabilitiesKHR& capabillities);

private:
	SwapchainSupportDetails supportDetails;

	VulkanAPI::QueueFamily queueFamily;
	VkSwapchainKHR swapchain;

	VkSurfaceKHR surface;
	Resolution resolution;

	uint32_t image_count;

	std::vector<VkImage> swapchainImages;

	VkDevice device;
	VkPhysicalDevice physicalDevice;
	struct GLFWwindow* window;
};