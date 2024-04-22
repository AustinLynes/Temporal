#include "Swapchain.h"

Swapchain::Swapchain(GLFWwindow* window, VkSurfaceKHR& surface, VulkanAPI::QueueFamily queueFamily, Resolution resolution, uint32_t images)
	:  window{window}, swapchain{ VK_NULL_HANDLE },  resolution{ resolution}, queueFamily{ queueFamily }
{
	Create(surface);
}

Swapchain::~Swapchain()
{
	-VulkanAPI::DestroySwapchain(swapchain);
}

TReturn Swapchain::Create(VkSurfaceKHR& surface)
{
	-VulkanAPI::CreateSwapchain(window, surface, queueFamily, VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,  3, swapchain);
	-VulkanAPI::GetSwapchainImages(swapchain, swapchainImages);
	return TReturn::SUCCESS;
}


VkSwapchainKHR Swapchain::Get()
{
	return swapchain;
}

VkImage Swapchain::GetImage(int idx)
{
	assert((idx < swapchainImages.size() && idx >= 0) && "Index must be non negative and less than or equal to the size of the images avaialable.");
	return swapchainImages[idx];
}

uint32_t Swapchain::ImageCount()
{
	return swapchainImages.size();
}

TReturn Swapchain::AquireNextImage(VkSemaphore& imageAvailableSemaphore, uint32_t& currentFrameIndex)
{
	-VulkanAPI::AquireNextImage(imageAvailableSemaphore, swapchain, currentFrameIndex);

	return TReturn::SUCCESS;
}





