#pragma once

#include <graphics/vulkan_api.h>
#include <map>

enum class MemoryType {
	Instance,
	PhysicalDevice,
	Device,
};

namespace Memory {


	std::map<MemoryType, std::vector<void*>> MemoryPool;
	/// DOX TEST!!!
	void* Allocate(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope scope) {

		MemoryType type = *(static_cast<MemoryType*>(pUserData));
		
		void* memory = malloc(size);

		switch (type)
		{
		case MemoryType::Instance:
		{
			// if this is the first time this is running..
			// the pools need to be initilized..
			if (MemoryPool.find(type) == MemoryPool.end())
				MemoryPool.insert({ MemoryType::Instance, {} });

			MemoryPool[MemoryType::Instance].push_back(memory);
		}
			break;
		case MemoryType::PhysicalDevice:
			break;
		case MemoryType::Device:
			break;
		default:
			break;
		}

		//Console::Info("Memory Allocated(0x", memory, "): ", size, " bytes");
		return memory;
	}

	void* Reallocate(void* pUserData, void* original, size_t size, size_t alignment, VkSystemAllocationScope scope) {
		void* memory = realloc(original, size);
		//Console::Info("Memory Reallocated(0x", memory, "): ", size, " bytes");
		return memory;
	}

	void Free(void* pUserData, void* pMemory) {
		//Console::Log("Memory Freed: 0x", pMemory);
		free(pMemory);
	}
}
