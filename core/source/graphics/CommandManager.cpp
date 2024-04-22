#include "CommandManager.h"



CommandManager::CommandManager(VulkanAPI::QueueFamily queueFamily)
{
	-VulkanAPI::CreateCommandPools(queueFamily, CommandPools);
	-VulkanAPI::AquireQueueHandles(queueFamily, CommandQueues);
}

CommandManager::~CommandManager()
{
	VulkanAPI::FreeCommandPoolBlock(CommandPools);
	VulkanAPI::FreeQueueHandles(CommandQueues);
}


VkCommandBuffer CommandManager::BeginSingleTimeCommand(VulkanAPI::CommandType type)
{
	VkCommandBuffer singleTimeBuffer;
	
	auto pool = GetPool(type);

	VulkanAPI::AllocateCommandBuffer(pool, singleTimeBuffer);

	VulkanAPI::BeginCommandBufferRecord(singleTimeBuffer);

	return singleTimeBuffer;
}

void CommandManager::EndSingleTimeCommand(VkCommandBuffer cmd, VulkanAPI::CommandType type, VkFence& fence)
{
	auto queue = GetQueue(type);
	auto pool = GetPool(type);

	VulkanAPI::EndCommandBufferRecord(cmd);
	VulkanAPI::SubmitCommandBuffer(cmd, queue, fence);
	VulkanAPI::FreeCommandBuffer(pool, cmd);

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
