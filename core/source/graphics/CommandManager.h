#pragma once

#include <graphics/vulkan_api.h>

namespace VulkanAPI {

}

class CommandManager {
public:
	CommandManager(VkDevice device, VulkanAPI::QueueFamily queuFamily);
	~CommandManager();


	VkCommandBuffer BeginSingleTimeCommand(VulkanAPI::CommandType type);
	void EndSingleTimeCommand(VkCommandBuffer cmd, VulkanAPI::CommandType type, VkFence& fence);

	VkQueue GetPresentQueue();

private:
	VkCommandPool GetPool(VulkanAPI::CommandType type);
	VkQueue GetQueue(VulkanAPI::CommandType type);
	VkCommandBuffer GetSingleTimeBuffer(VulkanAPI::CommandType type);
	void SetSingleTimeBuffer(VulkanAPI::CommandType type, const VkCommandBuffer& cmd);

	VkDevice device;
	VulkanAPI::CommandPoolBlock CommandPools;
	VulkanAPI::QueueHandleBlock CommandQueues;


};