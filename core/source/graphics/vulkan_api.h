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
		return TReturn::FAILURE;																					 \
	}																												 \
}	


#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <defs.h>



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

	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabillities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};


	// CORE
	TReturn CreateInstance();
	TReturn FreeInstance();

	TReturn GetPhysicalDevice();

	TReturn CreateDevice(QueueFamily queueFamily);
	TReturn FreeDevice();
	TReturn GetRequiredInfo();

	// QUEUES
	TReturn ReserveQueueFamily(QueueFamily& family, VkSurfaceKHR& surface);

	TReturn AquireQueueHandle(QueueFamily queueFamily, CommandType type, VkQueue& queue);
	TReturn AquireQueueHandles(QueueFamily queueFamily, QueueHandleBlock& block);
	TReturn FreeQueueHandles(QueueHandleBlock& block);
	// SURFACE
	TReturn CreateSurfaceGLFW(GLFWwindow* window, VkSurfaceKHR& surface);
	TReturn FreeSurface(VkSurfaceKHR& surface);

	// Swapchain, Rendering, and Framebuffers
	TReturn CreateRenderPass(std::vector<VkAttachmentDescription>& attachment, VkRenderPass& pass);
	TReturn DestroyRenderPass(VkRenderPass& renderPass);

	TReturn CreateFramebuffer(uint32_t width, uint32_t height, std::vector<VkImageView>& views, VkRenderPass& renderPass, VkFramebuffer& fbuffer);
	TReturn DestroyFramebuffer(VkFramebuffer& framebuffer);
	
	TReturn CreateSwapchain(GLFWwindow* window, VkSurfaceKHR surface, QueueFamily queueFamily, VkFormat format, VkColorSpaceKHR colorspace, uint32_t image_count, VkSwapchainKHR& swapchain);
	TReturn GetSwapchainImages(VkSwapchainKHR& swapchain, std::vector<VkImage>& images);
	TReturn DestroySwapchain(VkSwapchainKHR swapchain);
	TReturn AquireNextImage(VkSemaphore& imageAquiredSemaphore, VkSwapchainKHR swapchain, uint32_t& currentFrameIndex);

	// Pipelines 
	TReturn CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo info, VkPipeline& pipeline);
	TReturn DestroyPipeline(VkPipeline& pipeline);

	TReturn CreatePipelineLayout(std::vector<VkDescriptorSetLayout>& descriptors, std::vector<VkPushConstantRange>& pushConstants, VkPipelineLayout& layout);

	TReturn DestroyPipelineLayout(VkPipelineLayout& layout);

	// Command Pools
	TReturn CreateCommandPools(QueueFamily family, CommandPoolBlock& block);
	TReturn CreateCommandPool(uint32_t index, VkCommandPool& pool);
	TReturn FreeCommandPoolBlock(CommandPoolBlock& block);

	// command buffers
	TReturn AllocateCommandBuffer(VkCommandPool& pool, VkCommandBuffer& cmd);
	TReturn FreeCommandBuffer(VkCommandPool& pool, VkCommandBuffer& buffer);
	// recording
	TReturn BeginCommandBufferRecord(VkCommandBuffer& cmd);
	TReturn EndCommandBufferRecord(VkCommandBuffer& cmd);
	TReturn SubmitCommandBuffer(VkCommandBuffer& cmd, VkQueue& queue, VkFence& fence);

	TReturn CreateFenceBlock(FenceBlock& block);
	TReturn FreeFenceBlock(FenceBlock& block);
	TReturn CreateFenceSyncOjbect(bool signal, VkFence& fence);

	TReturn CreateSemaphoreSyncObject(VkSemaphore& semaphore);
	TReturn FreeSemaphoreBlock(SemaphoreBlock& block);
	TReturn CreateSemaphoreBlock(SemaphoreBlock& block);

	// RESOURCES && MEMORY
	TReturn CreateImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, VkImageType type, VkImage& image);
	TReturn DestroyImage(VkImage& image);

	TReturn CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkBuffer& buffer);

	TReturn AllocateImageMemory(VkImage& image, VkDeviceMemory& memory);
	TReturn FreeImageMemory(VkDeviceMemory& memory);

	TReturn CreateImageView(VkImage& image, VkFormat format, VkImageViewType type, VkImageView& view);
	TReturn DestroyImageView(VkImageView& view);

	TReturn CreateDescriptorPool(DescriptorPoolDesc desc, VkDescriptorPool& pool);
	TReturn CreateDescriptorSetLayout(DescriptorPoolDesc desc, VkDescriptorSetLayout& layout);

	TReturn AllocateDescriptorSet(VkDescriptorPool& pool, VkDescriptorSetLayout& layout, VkDescriptorSet& set);

	VkSurfaceFormatKHR SelectFormat(VkFormat format, VkColorSpaceKHR colorSpace);
	VkPresentModeKHR SelectPresentMode(VkPresentModeKHR presentMode);
	VkExtent2D SelectExtent(GLFWwindow* window);


	TReturn CreateShaderModule(std::vector<unsigned int> data, VkShaderModule& shaderModule);
	TReturn DestroyShaderModule(VkShaderModule& shaderModule);
}

