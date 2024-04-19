#include "Swapchain.h"

Swapchain::Swapchain(VkDevice device, VkPhysicalDevice physicalDevice, GLFWwindow*	window, VkSurfaceKHR surface, VulkanAPI::QueueFamily queueFamily, Resolution resolution, uint32_t images)
	: device{ device }, physicalDevice{ physicalDevice }, window{window}, surface { surface }, swapchain{ VK_NULL_HANDLE }, image_count{ images }, resolution{ resolution}, queueFamily{ queueFamily }
{
	Create();
}

Swapchain::~Swapchain()
{
	vkDestroySwapchainKHR(device, swapchain, nullptr);
}

void Swapchain::Create()
{

	supportDetails = GetSupportDetails();
	
	VkSurfaceFormatKHR format = SelectFormat(VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
	VkPresentModeKHR presentMode = SelectPresentMode(VK_PRESENT_MODE_FIFO_KHR);
	VkExtent2D imageExtent = SelectExtent(supportDetails.capabillities);

	image_count = std::clamp(image_count, supportDetails.capabillities.minImageCount + 1, supportDetails.capabillities.maxImageCount);

	bool shouldBeConcurent = (queueFamily.graphics.has_value() && queueFamily.present.has_value()) && (queueFamily.graphics.value() == queueFamily.present.value());
	std::vector<uint32_t> indices;
	if (queueFamily.graphics.has_value()) {
		indices.push_back(queueFamily.graphics.value());
	}
	if (queueFamily.present.has_value()) {
		indices.push_back(queueFamily.present.value());
	}


	VkSwapchainCreateInfoKHR info
	{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = surface,

		.minImageCount = image_count,
		.imageFormat = format.format,
		.imageColorSpace = format.colorSpace,
		.imageExtent = imageExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.imageSharingMode = shouldBeConcurent ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = (uint32_t)indices.size(),
		.pQueueFamilyIndices = indices.data(),
		.preTransform = supportDetails.capabillities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		
	};

	VK_CHECK(vkCreateSwapchainKHR(device, &info, nullptr, &swapchain));

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

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

SwapchainSupportDetails Swapchain::GetSupportDetails()
{
	SwapchainSupportDetails details;

	// Get Physical Device Surface Capabillities
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabillities));

	// Enumerate Surface Formats.
	uint32_t formatsCount;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, nullptr));
	details.formats.resize(formatsCount);
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatsCount, details.formats.data()));


	// enumerate presentation modes.
	uint32_t presentModesCount;
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr));
	details.presentModes.resize(presentModesCount);
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, details.presentModes.data()));


	return details;
}

VkSurfaceFormatKHR Swapchain::SelectFormat(VkFormat format, VkColorSpaceKHR colorSpace)
{
	auto formats = supportDetails.formats;

	for (auto f = formats.begin(); f != formats.end(); f++) {
		if (f->format == format && f->colorSpace == colorSpace) {
			return *f;
		}
	}
	

	return formats[0];
}

VkPresentModeKHR Swapchain::SelectPresentMode(VkPresentModeKHR presentMode)
{
	auto modes = supportDetails.presentModes;

	for (auto m = modes.begin(); m != modes.end(); m++)
	{
		if (*m == presentMode)
			return *m;
	}

	return modes[0];
}

VkExtent2D Swapchain::SelectExtent(const VkSurfaceCapabilitiesKHR& capabillities)
{
	VkExtent2D extent;

	if (capabillities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabillities.currentExtent;
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		extent.width = static_cast<uint32_t>(width);
		extent.height = static_cast<uint32_t>(height);
	}

	extent.width = std::clamp(extent.width, capabillities.minImageExtent.width, capabillities.maxImageExtent.width);
	extent.height = std::clamp(extent.height, capabillities.minImageExtent.height, capabillities.maxImageExtent.height);

	return extent;
}


