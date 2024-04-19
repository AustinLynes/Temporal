#include "vulkan_api.h"

#include <datastructures/datastructures_pch.h>
#include "memory/memory.h"


namespace VulkanAPI {

	// Util Function used for filtering sets of string
	std::unordered_set<std::string> filter(std::vector<std::string> available, std::vector<std::string> requested) {
		std::sort(available.begin(), available.end());
		std::sort(requested.begin(), requested.end());
		std::vector<std::string> result;
		std::set_intersection(available.begin(), available.end(), requested.begin(), requested.end(), std::back_inserter(result));

		return std::unordered_set<std::string>(result.begin(), result.end());
	}

	VkAllocationCallbacks CustomAllocCallbacks(void* pUserData) {
		static auto allocator = VkAllocationCallbacks{
			.pUserData = pUserData,
			.pfnAllocation = Memory::Allocate,
			.pfnReallocation = Memory::Reallocate,
			.pfnFree = Memory::Free,
		};
		return allocator;
	}


	VkInstance CreateInstance(std::vector<std::string> layers, std::vector<std::string> extensions) {

		VkInstance inst;

		uint32_t layerCount = -1;
		VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
		std::vector<VkLayerProperties> found_layers(layerCount);
		VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, found_layers.data()));

		std::vector<std::string> available_layers;

		std::transform(found_layers.begin(), found_layers.end(), std::back_inserter(available_layers), [](const VkLayerProperties& l) {return l.layerName; });

		auto filtered = filter(available_layers, layers);

		std::unordered_set<std::string> avialable_extensions;

		for (auto i = filtered.begin(); i != filtered.end(); i++)
		{

			uint32_t count = 0;
			VK_CHECK(vkEnumerateInstanceExtensionProperties(i->c_str(), &count, nullptr));
			std::vector<VkExtensionProperties> extensions(count);

			VK_CHECK(vkEnumerateInstanceExtensionProperties(i->c_str(), &count, extensions.data()));

			for (auto e = extensions.begin(); e != extensions.end(); e++)
			{
				avialable_extensions.insert(e->extensionName);
			}

		}

		for (auto i = extensions.begin(); i != extensions.end(); i++) {
			avialable_extensions.insert(i->c_str());
			std::cout << "extension available: \t" << *i << std::endl;
		}


		std::vector<const char*> enabled_layers;
		std::transform(layers.begin(), layers.end(), std::back_inserter(enabled_layers), [](const std::string& name) {return name.c_str(); });

		std::vector<const char*> enabled_extensions;
		std::transform(avialable_extensions.begin(), avialable_extensions.end(), std::back_inserter(enabled_extensions), [](const std::string& name) {return name.c_str(); });

		VkApplicationInfo app{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pNext = nullptr,
			.pApplicationName = "Temporal",
			.pEngineName = "Temporal",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_3,
		};

		VkInstanceCreateInfo info
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pApplicationInfo = &app,
			.enabledLayerCount = (uint32_t)enabled_layers.size(),
			.ppEnabledLayerNames = enabled_layers.data(),
			.enabledExtensionCount = (uint32_t)enabled_extensions.size(),
			.ppEnabledExtensionNames = enabled_extensions.data(),
		};

#if _DEBUG
		{
			std::cout << "Enabled Layers:\n";
			for (auto i = enabled_layers.begin(); i < enabled_layers.end(); i++)
			{
				std::cout << " \t" << *i << std::endl;
			}
			std::cout << std::endl;
			std::cout << "Enabled Extensions:\n";
			for (auto i = enabled_extensions.begin(); i < enabled_extensions.end(); i++)
			{
				std::cout << "\t" << *i << std::endl;
			}
		}
#endif
		auto type = MemoryType::Instance;
		auto allocator = CustomAllocCallbacks((void*)&type);
		VK_CHECK(vkCreateInstance(&info, &allocator, &inst));


		return inst;
	}

	void FreeInstance(VkInstance& instance)
	{
		if (instance == VK_NULL_HANDLE)
			return;

		auto type = MemoryType::Instance;
		auto allocator = CustomAllocCallbacks((void*)&type);
		vkDestroyInstance(instance, &allocator);
	}

	VkPhysicalDevice GetPhysicalDevice(VkInstance instance) {

		struct PhysicalDeviceEntry
		{
			VkPhysicalDevice device;
			VkPhysicalDeviceProperties props;
			VkPhysicalDeviceFeatures features;
			int score;
		};

		uint32_t device_count = -1;
		VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, nullptr));

		std::vector<VkPhysicalDevice> devices(device_count);
		VK_CHECK(vkEnumeratePhysicalDevices(instance, &device_count, devices.data()));

		std::vector<PhysicalDeviceEntry> scores;

		for (auto device = devices.begin(); device != devices.end(); device++) {
			int score = 0;


			VkPhysicalDeviceProperties props;
			VkPhysicalDeviceFeatures features;

			vkGetPhysicalDeviceProperties(*device, &props);
			vkGetPhysicalDeviceFeatures(*device, &features);

			if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				score += 5000;
			}


			scores.push_back({ *device,  props, features, score });
		}

		std::sort(scores.begin(), scores.end(), [](PhysicalDeviceEntry a, PhysicalDeviceEntry b) { return a.score > b.score;  });

		std::cout << "Found Physical Devices.\n";
		for (auto entry = scores.begin(); entry != scores.end(); entry++)
		{
			std::cout << "\t: " << entry->props.deviceName << ", score: " << entry->score << std::endl;
		}

		VkPhysicalDevice device = scores[0].device;

		std::cout << "Selected Device: " << scores[0].props.deviceName << std::endl;

		return device;
	}

	VkDevice CreateDevice(VkInstance instance, VkPhysicalDevice physicalDevice, std::vector<std::string> enabled_layers, std::vector<std::string> enabled_extensions, QueueFamily queueFamily) {
		if (instance == VK_NULL_HANDLE || physicalDevice == VK_NULL_HANDLE)
			return VK_NULL_HANDLE;

		std::vector<const char*> layers;
		std::transform(enabled_layers.begin(), enabled_layers.end(), std::back_inserter(layers), [](const std::string& str) {return str.c_str(); });

		std::vector<const char*> extensions;
		std::transform(enabled_extensions.begin(), enabled_extensions.end(), std::back_inserter(extensions), [](const std::string& str) {return str.c_str(); });


		VkDevice device;

		float priority = 1;

		auto queues = queueFamily.GetCreateInfo();

		VkPhysicalDeviceFeatures deviceFeatures{};

		vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

		VkDeviceCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.queueCreateInfoCount = (uint32_t)queues.size(),
			.pQueueCreateInfos = queues.data(),
			.enabledExtensionCount = (uint32_t)extensions.size(),
			.ppEnabledExtensionNames = extensions.data(),
			.pEnabledFeatures = &deviceFeatures,

		};

		vkCreateDevice(physicalDevice, &info, nullptr, &device);


		return device;

	}

	void FreeDevice(VkDevice& device)
	{
		if (device == VK_NULL_HANDLE)
			return;

		vkDestroyDevice(device, nullptr);
	}

	QueueHandleBlock AquireQueueHandles(VkDevice device, QueueFamily queueFamily) {
		QueueHandleBlock block{ VK_NULL_HANDLE };

		if (queueFamily.graphics.has_value())
			block.Graphics = AquireQueueHandle(device, queueFamily, CommandType::Graphics);
		if (queueFamily.present.has_value())
			block.Present = AquireQueueHandle(device, queueFamily, CommandType::Present);
		if (queueFamily.compute.has_value())
			block.Compute = AquireQueueHandle(device, queueFamily, CommandType::Compute);
		if (queueFamily.transfer.has_value())
			block.Transfer = AquireQueueHandle(device, queueFamily, CommandType::Transfer);
		if (queueFamily.sparse_binding.has_value())
			block.SparseBinding = AquireQueueHandle(device, queueFamily, CommandType::SparseBinding);

		return block;
	}

	void FreeQueueHandles(VkDevice device, QueueHandleBlock& block)
	{
		if(block.Graphics)
			vkQueueWaitIdle(block.Graphics);		
		if (block.Compute)
			vkQueueWaitIdle(block.Compute);
		if (block.Present)
			vkQueueWaitIdle(block.Present);
		if (block.Transfer)
			vkQueueWaitIdle(block.Transfer);
		if (block.SparseBinding)
			vkQueueWaitIdle(block.SparseBinding);
	}

	VkQueue AquireQueueHandle(VkDevice device, QueueFamily queueFamily, CommandType type) {

		VkQueue queue{ VK_NULL_HANDLE };

		switch (type)
		{
		case CommandType::Graphics:
			vkGetDeviceQueue(device, queueFamily.graphics.value(), 0, &queue);
			break;
		case CommandType::Present:
			vkGetDeviceQueue(device, queueFamily.present.value(), 0, &queue);
			break;
		case CommandType::Compute:
			vkGetDeviceQueue(device, queueFamily.compute.value(), 0, &queue);
			break;
		case CommandType::Transfer:
			vkGetDeviceQueue(device, queueFamily.transfer.value(), 0, &queue);
			break;
		case CommandType::SparseBinding:
			vkGetDeviceQueue(device, queueFamily.sparse_binding.value(), 0, &queue);
			break;
		}

		return queue;
	}

	VkSurfaceKHR CreateSurfaceGLFW(VkInstance instance, GLFWwindow* window)
	{
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		VkWin32SurfaceCreateInfoKHR info{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.hinstance = GetModuleHandle(NULL),
			.hwnd = (HWND)glfwGetWin32Window(window),
		};

		VK_CHECK(vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface));

		return surface;
	}

	void FreeSurface(VkInstance instance, VkSurfaceKHR& surface) {
	
		if (instance == VK_NULL_HANDLE || surface == VK_NULL_HANDLE)
			return;

		vkDestroySurfaceKHR(instance, surface, nullptr);
	}

	CommandPoolBlock CreateCommandPools(VkDevice device, QueueFamily family)
	{
		CommandPoolBlock block{ VK_NULL_HANDLE };

		if (family.graphics.has_value())
			block.Graphics = CreateCommandPool(device, family.graphics.value());

		if (family.present.has_value())
			block.Present = CreateCommandPool(device, family.present.value());

		if (family.compute.has_value())
			block.Compute = CreateCommandPool(device, family.compute.value());

		if (family.transfer.has_value())
			block.Transfer = CreateCommandPool(device, family.transfer.value());

		if (family.sparse_binding.has_value())
			block.SparseBinding = CreateCommandPool(device, family.sparse_binding.value());


		return block;
	}

	VkCommandPool CreateCommandPool(VkDevice device, uint32_t index) {

		VkCommandPool pool = VK_NULL_HANDLE;
		// Create Command Pool
		VkCommandPoolCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = index
		};

		VK_CHECK(vkCreateCommandPool(device, &info, nullptr, &pool));

		return pool;
	}

	void FreeCommandPoolBlock(VkDevice device, CommandPoolBlock& block)
	{
		if (block.Graphics)
			vkDestroyCommandPool(device, block.Graphics, nullptr);
		if (block.Compute)
			vkDestroyCommandPool(device, block.Compute, nullptr);
		if (block.Present)
			vkDestroyCommandPool(device, block.Present, nullptr);
		if (block.Transfer)
			vkDestroyCommandPool(device, block.Transfer, nullptr);
		if (block.SparseBinding)
			vkDestroyCommandPool(device, block.SparseBinding, nullptr);
	}

	FenceBlock CreateFenceBlock(VkDevice device)
	{
		FenceBlock block;

		block.Drawing = CreateFenceSyncOjbect(device);
		block.Presenting = CreateFenceSyncOjbect(device);

		return block;
	}

	void FreeFenceBlock(VkDevice device, FenceBlock& block)
	{
		if (block.Drawing != VK_NULL_HANDLE && vkWaitForFences(device, 1, &block.Drawing, VK_TRUE, UINT64_MAX) == VK_SUCCESS)
			vkDestroyFence(device, block.Drawing, nullptr);
		if (block.Presenting != VK_NULL_HANDLE && vkWaitForFences(device, 1, &block.Presenting, VK_TRUE, UINT64_MAX) == VK_SUCCESS) 
			vkDestroyFence(device, block.Presenting, nullptr);
	}

	VkFence CreateFenceSyncOjbect(VkDevice dev, bool signal)
	{
		VkFence fence = VK_NULL_HANDLE;

		VkFenceCreateFlags flags = 0;

		if (signal)
			flags |= VK_FENCE_CREATE_SIGNALED_BIT;

		VkFenceCreateInfo fenceInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags
		};

		VK_CHECK(vkCreateFence(dev, &fenceInfo, nullptr, &fence));

		return fence;

	}

	VkSemaphore CreateSemaphoreSyncObject(VkDevice dev)
	{
		VkSemaphore semaphore = VK_NULL_HANDLE;

		VkSemaphoreCreateInfo info
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
		};

		VK_CHECK(vkCreateSemaphore(dev, &info, nullptr, &semaphore));

		return semaphore;
	}

	void FreeSemaphoreBlock(VkDevice device, SemaphoreBlock& block)
	{
		if (block.ImageAvailable != VK_NULL_HANDLE)
			vkDestroySemaphore(device, block.ImageAvailable, nullptr);
	}

	SemaphoreBlock CreateSemaphoreBlock(VkDevice device)
	{
		SemaphoreBlock block;
		block.ImageAvailable = CreateSemaphoreSyncObject(device);
		return block;
	}

	QueueFamily ReserveQueueFamily(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {

		QueueFamily family;

		uint32_t count = -1;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);

		std::vector<VkQueueFamilyProperties> queues(count);

		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queues.data());

		int i = 0;
		for (auto q = queues.begin(); q != queues.end(); q++) {

			if (q->queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				family.graphics = i;
			}
			else if (q->queueFlags & VK_QUEUE_COMPUTE_BIT) {
				family.compute = i;
			}
			else if (q->queueFlags & VK_QUEUE_TRANSFER_BIT) {
				family.transfer = i;
			}
			else if (q->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
				family.sparse_binding = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport) {
				family.present = i;
			}


			i++;

		}

#if _DEBUG
		std::cout << "Queue Family:\n"
			<< "\t[" << (family.graphics.has_value() ? "X" : " ") << "]" << " Graphics\n" << "\t\t id: " << (family.graphics.has_value() ? std::to_string(family.graphics.value()) : " ") << "\n"
			<< "\t[" << (family.present.has_value() ? "X" : " ") << "]" << " Present\n" << "\t\t id: " << (family.present.has_value() ? std::to_string(family.present.value()) : " ") << "\n"
			<< "\t[" << (family.compute.has_value() ? "X" : " ") << "]" << " Compute\n" << "\t\t id: " << (family.compute.has_value() ? std::to_string(family.compute.value()) : " ") << "\n"
			<< "\t[" << (family.transfer.has_value() ? "X" : " ") << "]" << " Transfer\n" << "\t\t id: " << (family.transfer.has_value() ? std::to_string(family.transfer.value()) : " ") << "\n"
			<< "\t[" << (family.sparse_binding.has_value() ? "X" : " ") << "]" << " Sparse Binding\n" << "\t\t id: " << (family.sparse_binding.has_value() ? std::to_string(family.sparse_binding.value()) : " ") << "\n"
			<< "\n";
#endif

		return family;
	}

}
