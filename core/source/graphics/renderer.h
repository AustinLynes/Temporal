#pragma once

#include <iostream>
#include <functional>
#include <string>
#include <algorithm>
#include <set>
#include <optional>

#include <windows.h>

#include <cassert>
#include <unordered_set>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <volk/volk.h>

enum class CommandType {
	Graphics,
	Present,
	Compute,
	Transfer,
	SparseBinding
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;
	std::optional<uint32_t> compute;
	std::optional<uint32_t> transfer;
	std::optional<uint32_t> sparse_binding;


	std::vector<VkDeviceQueueCreateInfo> GetCreateInfo() {

		float priority = 1.0f;

		std::vector<VkDeviceQueueCreateInfo> queues;

		if (graphics.has_value())
			queues.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = graphics.value(),
				.queueCount = 1,
				.pQueuePriorities = &priority
				});
		if (compute.has_value())
			queues.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = compute.value(),
				.queueCount = 1,
				.pQueuePriorities = &priority
				});
		if (transfer.has_value())
			queues.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = transfer.value(),
				.queueCount = 1,
				.pQueuePriorities = &priority
				});

		if (sparse_binding.has_value())
			queues.push_back({
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.queueFamilyIndex = sparse_binding.value(),
				.queueCount = 1,
				.pQueuePriorities = &priority
				});


		return queues;
	};

};

struct FenceBlock {
	VkFence Drawing;
	VkFence Presenting;
};


enum class OperationType {
	Drawing,
	Presenting
};

enum class PipelineType {
	Graphics, 
	Compute,
};

struct CommandPoolBlock {
	VkCommandPool Graphics = VK_NULL_HANDLE;
	VkCommandPool Present = VK_NULL_HANDLE;
	VkCommandPool Compute = VK_NULL_HANDLE;
	VkCommandPool Transfer = VK_NULL_HANDLE;
	VkCommandPool SparseBinding = VK_NULL_HANDLE;
};

struct QueueHandleBlock {
	VkQueue Graphics = VK_NULL_HANDLE;
	VkQueue Present = VK_NULL_HANDLE;
	VkQueue Compute = VK_NULL_HANDLE;
	VkQueue Transfer = VK_NULL_HANDLE;
	VkQueue SparseBinding = VK_NULL_HANDLE;
};

#define VK_CHECK(res){																								 \
	if ((res != VK_SUCCESS)) {																						 \
		std::cerr << "vulkan error -> File: " << __FILE__ << " Line: " << __LINE__ << "Result: " << res << std::endl;\
		assert(false);}	}																							 \



class Renderer {

public:
	Renderer();
	~Renderer();

	bool Initilize(GLFWwindow* window);
	void Cleanup();

	void RenderFrame();
	void PresentFrame();

private:
	// MISC
	void GetRequiredInfo();
	// CORE
	VkInstance CreateInstance();
	VkPhysicalDevice GetPhysicalDevice();
	VkDevice CreateDevice();
	// QUEUES
	QueueFamilyIndices ReserveQueueFamily();
	VkQueue AquireQueueHandle(CommandType type);
	QueueHandleBlock AquireQueueHandles();
	// SURFACE
	VkSurfaceKHR CreateSurface(GLFWwindow* window);

	// Command Pools
	CommandPoolBlock CreateCommandPools(QueueFamilyIndices family);
	VkCommandPool CreateCommandPool(uint32_t index);

	FenceBlock CreateFenceBlock();
	VkFence CreateFence();

	VkPipeline CreatePipeline(VkDevice device, PipelineType type);


};