#pragma once

#include <pch.h>

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
																						 \
#define VK_CHECK(res){																								 \
	if ((res != VK_SUCCESS)) {																						 \
		std::cerr << "vulkan error -> File: " << __FILE__ << " Line: " << __LINE__ << "Result: " << res << std::endl;\
		assert(false);}	}	


#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


namespace VulkanAPI {

	enum class CommandType {
		Graphics,
		Present,
		Compute,
		Transfer,
		SparseBinding
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
	struct CommandBufferBlock {
		VkCommandBuffer Graphics = VK_NULL_HANDLE;
		VkCommandBuffer Present = VK_NULL_HANDLE;
		VkCommandBuffer Compute = VK_NULL_HANDLE;
		VkCommandBuffer Transfer = VK_NULL_HANDLE;
		VkCommandBuffer SparseBinding = VK_NULL_HANDLE;
	};

	struct FenceBlock {
		VkFence Drawing;
		VkFence Presenting;
	};
	struct SemaphoreBlock {
		VkSemaphore ImageAvailable;
	};

	struct QueueHandleBlock {
		VkQueue Graphics = VK_NULL_HANDLE;
		VkQueue Present = VK_NULL_HANDLE;
		VkQueue Compute = VK_NULL_HANDLE;
		VkQueue Transfer = VK_NULL_HANDLE;
		VkQueue SparseBinding = VK_NULL_HANDLE;
	};

	struct QueueFamily {
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

	struct DescriptorPoolDesc {
		std::optional<uint32_t> numRequestedSamplers;
		std::optional<uint32_t> numRequestedCombinedSamplers;
		std::optional<uint32_t> numRequestedUniformBuffers;
		std::optional<uint32_t> numRequestedStorageBuffers;

		uint32_t total(){
			uint32_t sum = 0;
			
			if (numRequestedSamplers.has_value())
				sum += numRequestedSamplers.value();
			
			if (numRequestedCombinedSamplers.has_value())
				sum += numRequestedCombinedSamplers.value();
			
			if (numRequestedStorageBuffers.has_value())
				sum += numRequestedStorageBuffers.value();

			if (numRequestedUniformBuffers.has_value())
				sum += numRequestedUniformBuffers.value();

			return sum;
		}
	};

	// CORE
	VkInstance CreateInstance(std::vector<std::string> layers, std::vector<std::string> extensions);
	void FreeInstance(VkInstance& instance);

	VkPhysicalDevice GetPhysicalDevice(VkInstance instance);

	VkDevice CreateDevice(VkInstance instance, VkPhysicalDevice physicalDevice, std::vector<std::string> enabled_layers, std::vector<std::string> enabled_extensions, VulkanAPI::QueueFamily queueFamily);
	void FreeDevice(VkDevice& device);

	// QUEUES
	QueueFamily ReserveQueueFamily(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

	VkQueue AquireQueueHandle(VkDevice device, QueueFamily queueFamily, CommandType type);
	QueueHandleBlock AquireQueueHandles(VkDevice device, QueueFamily queueFamily);
	void FreeQueueHandles(VkDevice device, QueueHandleBlock& block);
	// SURFACE
	VkSurfaceKHR CreateSurfaceGLFW(VkInstance instance, GLFWwindow* window);
	void FreeSurface(VkInstance instance, VkSurfaceKHR& surface);

	// Command Pools
	CommandPoolBlock CreateCommandPools(VkDevice device, QueueFamily family);
	VkCommandPool CreateCommandPool(VkDevice device, uint32_t index);
	void FreeCommandPoolBlock(VkDevice device, CommandPoolBlock& block);

	FenceBlock CreateFenceBlock(VkDevice device);
	void FreeFenceBlock(VkDevice device, FenceBlock& block);
	VkFence CreateFenceSyncOjbect(VkDevice device, bool signal = false);

	VkSemaphore CreateSemaphoreSyncObject(VkDevice device);
	void FreeSemaphoreBlock(VkDevice device, SemaphoreBlock& block);
	SemaphoreBlock CreateSemaphoreBlock(VkDevice device);

	// RESOURCES && MEMORY
	VkImage CreateImage(VkDevice device, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageType type);
	VkDeviceMemory AllocateImageMemory(VkDevice device, VkImage& image);
	VkImageView CreateImageView(VkDevice device, VkImage& image, VkFormat format, VkImageViewType type);

	VkDescriptorPool CreateDescriptorPool(VkDevice device, DescriptorPoolDesc desc);
	VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device, DescriptorPoolDesc desc);

	VkDescriptorSet AllocateDescriptorSet(VkDevice device, VkDescriptorPool& pool, VkDescriptorSetLayout& layout);
}

