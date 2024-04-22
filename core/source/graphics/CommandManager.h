#pragma once

#include <graphics/vulkan_api.h>

class CommandManager {
public:
	CommandManager(VulkanAPI::QueueFamily queuFamily);
	~CommandManager();


	VkCommandBuffer BeginSingleTimeCommand(VulkanAPI::CommandType type);
	void EndSingleTimeCommand(VkCommandBuffer cmd, VulkanAPI::CommandType type, VkFence& fence);

	VkQueue GetPresentQueue();

private:
	VkCommandPool GetPool(VulkanAPI::CommandType type);
	VkQueue GetQueue(VulkanAPI::CommandType type);

	VulkanAPI::CommandPoolBlock CommandPools;
	VulkanAPI::QueueHandleBlock CommandQueues;


};