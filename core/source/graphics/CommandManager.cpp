#include "CommandManager.h"



CommandManager::CommandManager(VkDevice device, VulkanAPI::QueueFamily queueFamily)
	: device{device}
{
	CommandPools = VulkanAPI::CreateCommandPools(device, queueFamily);
	CommandQueues = VulkanAPI::AquireQueueHandles(device, queueFamily);

}

CommandManager::~CommandManager()
{
	VulkanAPI::FreeCommandPoolBlock(device, CommandPools);
	VulkanAPI::FreeQueueHandles(device, CommandQueues);
}


VkCommandBuffer CommandManager::BeginSingleTimeCommand(VulkanAPI::CommandType type)
{
	VkCommandBufferAllocateInfo info
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = GetPool(type),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer singleTimeBuffer;

	VK_CHECK(vkAllocateCommandBuffers(device, &info, &singleTimeBuffer));

	VkCommandBufferBeginInfo begin{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
	};

	VK_CHECK(vkBeginCommandBuffer(singleTimeBuffer, &begin));

	return singleTimeBuffer;
}

void CommandManager::EndSingleTimeCommand(VkCommandBuffer cmd, VulkanAPI::CommandType type, VkFence& fence)
{
	VK_CHECK(vkEndCommandBuffer(cmd));

	VK_CHECK(vkResetFences(device, 1, &fence));

	VkSubmitInfo info
	{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd,
	};

	VK_CHECK(vkQueueSubmit(GetQueue(type), 1, &info, fence));
	
	VK_CHECK(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));

	vkFreeCommandBuffers(device, GetPool(type), 1, &cmd);

}

VkQueue CommandManager::GetPresentQueue()
{
	return CommandQueues.Present;
}

VkCommandPool CommandManager::GetPool(VulkanAPI::CommandType type)
{
	VkCommandPool pool = VK_NULL_HANDLE;

	switch (type)
	{
	case VulkanAPI::CommandType::Graphics:
		pool = CommandPools.Graphics;
		break;
	case VulkanAPI::CommandType::Present:
		pool = CommandPools.Present;
		break;
	case VulkanAPI::CommandType::Compute:
		pool = CommandPools.Compute;
		break;
	case VulkanAPI::CommandType::Transfer:
		pool = CommandPools.Transfer;
		break;
	case VulkanAPI::CommandType::SparseBinding:
		pool = CommandPools.SparseBinding;
		break;

	}

	return pool;
}

VkQueue CommandManager::GetQueue(VulkanAPI::CommandType type)
{
	VkQueue queue = VK_NULL_HANDLE;

	switch (type)
	{
	case VulkanAPI::CommandType::Graphics:
		queue = CommandQueues.Graphics;
		break;
	case VulkanAPI::CommandType::Present:
		queue = CommandQueues.Present;
		break;
	case VulkanAPI::CommandType::Compute:
		queue = CommandQueues.Compute;
		break;
	case VulkanAPI::CommandType::Transfer:
		queue = CommandQueues.Transfer;
		break;
	case VulkanAPI::CommandType::SparseBinding:
		queue = CommandQueues.SparseBinding;
		break;

	}

	return queue;
}
