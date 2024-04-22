#include "vulkan_api.h"

#include <datastructures/datastructures_pch.h>
#include "memory/memory.h"


namespace VulkanAPI {

	namespace vk
	{
		VkInstance Instance = { VK_NULL_HANDLE };
		VkPhysicalDevice PhysicalDevice = { VK_NULL_HANDLE };
		VkDevice Device = { VK_NULL_HANDLE };

		std::vector<std::string> layers = {};
		std::vector<std::string> instance_extensions = {};
		std::vector<std::string> device_extensions = {};

		SwapchainSupportDetails swachainSupportDetails{};

	} // GLOBALS

	// Util Function used for filtering sets of string
	std::unordered_set<std::string> filter(std::vector<std::string> available, std::vector<std::string> requested) {
		std::sort(available.begin(), available.end());
		std::sort(requested.begin(), requested.end());
		std::vector<std::string> result;
		std::set_intersection(available.begin(), available.end(), requested.begin(), requested.end(), std::back_inserter(result));

		return std::unordered_set<std::string>(result.begin(), result.end());
	}

	/// TODO! 
	/// CUSTOM ALLOCATION CALLBACKS.
	VkAllocationCallbacks CustomAllocCallbacks(void* pUserData) {
		static auto allocator = VkAllocationCallbacks{
			.pUserData = pUserData,
			.pfnAllocation = Memory::Allocate,
			.pfnReallocation = Memory::Reallocate,
			.pfnFree = Memory::Free,
		};
		return allocator;
	}


	TReturn GetRequiredInfo()
	{
#ifdef _DEBUG 
		bool isValidationEnabled = true;
#else
		bool isValidationEnabled = false;
#endif


		if (isValidationEnabled)
		{
			vk::layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		// load required extension for use with GLFW
		uint32_t extCount = -1;
		auto glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&extCount);
		for (size_t i = 0; i < extCount; i++)
		{
			vk::instance_extensions.push_back(glfwRequiredExtensions[i]);
		}

		// load requried device extensions

		vk::device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	
		return TReturn::SUCCESS;
	}




	TReturn CreateInstance() {
		TReturn res = TReturn::SUCCESS;

		uint32_t layerCount = -1;
		VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
		std::vector<VkLayerProperties> found_layers(layerCount);
		VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, found_layers.data()));

		std::vector<std::string> available_layers;

		std::transform(found_layers.begin(), found_layers.end(), std::back_inserter(available_layers), [](const VkLayerProperties& l) {return l.layerName; });

		auto filtered = filter(available_layers, vk::layers);

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

		for (auto i = vk::instance_extensions.begin(); i != vk::instance_extensions.end(); i++) {
			avialable_extensions.insert(i->c_str());
			std::cout << "extension available: \t" << *i << std::endl;
		}


		std::vector<const char*> enabled_layers;
		std::transform(vk::layers.begin(), vk::layers.end(), std::back_inserter(enabled_layers), [](const std::string& name) {return name.c_str(); });

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

		VK_CHECK(vkCreateInstance(&info, &allocator, &vk::Instance));

		return TReturn::SUCCESS;
	}



	TReturn FreeInstance()
	{
		if (vk::Instance == VK_NULL_HANDLE)
			return TReturn::FAILURE;

		auto type = MemoryType::Instance;
		auto allocator = CustomAllocCallbacks((void*)&type);
		vkDestroyInstance(vk::Instance, &allocator);

		return TReturn::SUCCESS;
	}

	TReturn GetPhysicalDevice() {

		struct PhysicalDeviceEntry
		{
			VkPhysicalDevice device;
			VkPhysicalDeviceProperties props;
			VkPhysicalDeviceFeatures features;
			int score;
		};

		uint32_t device_count = -1;
		VK_CHECK(vkEnumeratePhysicalDevices(vk::Instance, &device_count, nullptr));

		std::vector<VkPhysicalDevice> devices(device_count);
		VK_CHECK(vkEnumeratePhysicalDevices(vk::Instance, &device_count, devices.data()));

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

		vk::PhysicalDevice = scores[0].device;

		std::cout << "Selected Device: " << scores[0].props.deviceName << std::endl;

		return TReturn::SUCCESS;
	}

	TReturn CreateDevice(QueueFamily queueFamily) {
		if (vk::Instance == VK_NULL_HANDLE || vk::PhysicalDevice == VK_NULL_HANDLE)
			return TReturn::FAILURE;

		std::vector<const char*> layers;
		std::transform(vk::layers.begin(), vk::layers.end(), std::back_inserter(layers), [](const std::string& str) {return str.c_str(); });

		std::vector<const char*> extensions;
		std::transform(vk::device_extensions.begin(), vk::device_extensions.end(), std::back_inserter(extensions), [](const std::string& str) {return str.c_str(); });


		VkDevice device;

		float priority = 1;

		auto queues = queueFamily.GetCreateInfo();

		VkPhysicalDeviceFeatures deviceFeatures{};

		vkGetPhysicalDeviceFeatures(vk::PhysicalDevice, &deviceFeatures);

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

		vkCreateDevice(vk::PhysicalDevice, &info, nullptr, &vk::Device);

		return TReturn::SUCCESS;

	}

	TReturn FreeDevice()
	{
		if (vk::Device == VK_NULL_HANDLE)
			return TReturn::FAILURE;

		vkDestroyDevice(vk::Device, nullptr);

		return TReturn::SUCCESS;
	}


	TReturn ReserveQueueFamily(QueueFamily& family, VkSurfaceKHR& surface) {

		family = {};

		uint32_t count = -1;
		vkGetPhysicalDeviceQueueFamilyProperties(vk::PhysicalDevice, &count, nullptr);

		std::vector<VkQueueFamilyProperties> queues(count);

		vkGetPhysicalDeviceQueueFamilyProperties(vk::PhysicalDevice, &count, queues.data());

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
			vkGetPhysicalDeviceSurfaceSupportKHR(vk::PhysicalDevice, i, surface, &presentSupport);

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

		return TReturn::SUCCESS;
	}


	TReturn AquireQueueHandles(QueueFamily queueFamily, QueueHandleBlock& block) {
		block = { VK_NULL_HANDLE };

		if (queueFamily.graphics.has_value())
			- AquireQueueHandle(queueFamily, CommandType::Graphics, block.Graphics);
		if (queueFamily.present.has_value())
			- AquireQueueHandle(queueFamily, CommandType::Present, block.Present);
		if (queueFamily.compute.has_value())
			- AquireQueueHandle(queueFamily, CommandType::Compute, block.Compute);
		if (queueFamily.transfer.has_value())
			- AquireQueueHandle(queueFamily, CommandType::Transfer, block.Transfer);
		if (queueFamily.sparse_binding.has_value())
			- AquireQueueHandle(queueFamily, CommandType::SparseBinding, block.SparseBinding);

		return TReturn::SUCCESS;
	}

	TReturn FreeQueueHandles(QueueHandleBlock& block)
	{
		if (block.Graphics)
			vkQueueWaitIdle(block.Graphics);
		if (block.Compute)
			vkQueueWaitIdle(block.Compute);
		if (block.Present)
			vkQueueWaitIdle(block.Present);
		if (block.Transfer)
			vkQueueWaitIdle(block.Transfer);
		if (block.SparseBinding)
			vkQueueWaitIdle(block.SparseBinding);

		return TReturn::SUCCESS;
	}

	TReturn AquireQueueHandle(QueueFamily queueFamily, CommandType type, VkQueue& queue) {

		queue = { VK_NULL_HANDLE };

		switch (type)
		{
		case CommandType::Graphics:
			vkGetDeviceQueue(vk::Device, queueFamily.graphics.value(), 0, &queue);
			break;
		case CommandType::Present:
			vkGetDeviceQueue(vk::Device, queueFamily.present.value(), 0, &queue);
			break;
		case CommandType::Compute:
			vkGetDeviceQueue(vk::Device, queueFamily.compute.value(), 0, &queue);
			break;
		case CommandType::Transfer:
			vkGetDeviceQueue(vk::Device, queueFamily.transfer.value(), 0, &queue);
			break;
		case CommandType::SparseBinding:
			vkGetDeviceQueue(vk::Device, queueFamily.sparse_binding.value(), 0, &queue);
			break;
		}

		return TReturn::SUCCESS;
	}

	TReturn CreateSurfaceGLFW(GLFWwindow* window, VkSurfaceKHR& surface)
	{
		surface = VK_NULL_HANDLE;

		VkWin32SurfaceCreateInfoKHR info{
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.hinstance = GetModuleHandle(NULL),
			.hwnd = (HWND)glfwGetWin32Window(window),
		};

		VK_CHECK(vkCreateWin32SurfaceKHR(vk::Instance, &info, nullptr, &surface));

		return TReturn::SUCCESS;
	}

	TReturn FreeSurface(VkSurfaceKHR& surface) {

		if (vk::Instance == VK_NULL_HANDLE)
			return TReturn::FAILURE;


		vkDestroySurfaceKHR(vk::Instance, surface, nullptr);

		return TReturn::SUCCESS;
	}

	TReturn CreateRenderPass(std::vector<VkAttachmentDescription>& attachments, VkRenderPass& pass)
	{
		pass = VK_NULL_HANDLE;

		std::vector<VkAttachmentReference> attachmentRefs{
			// Pass 0 ref
			{
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			}
		};

		std::vector<VkSubpassDescription> subpasses
		{
			{
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = (uint32_t)attachmentRefs.size(),
				.pColorAttachments = attachmentRefs.data()
			}
		};


		VkRenderPassCreateInfo renderPassCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,

			.attachmentCount = (uint32_t)attachments.size(),
			.pAttachments = attachments.data(),

			.subpassCount = (uint32_t)subpasses.size(),
			.pSubpasses = subpasses.data(),

		};

		VK_CHECK(vkCreateRenderPass(vk::Device, &renderPassCreateInfo, nullptr, &pass));

		return TReturn::SUCCESS;
	}

	TReturn DestroyRenderPass(VkRenderPass& renderPass)
	{
		vkDestroyRenderPass(vk::Device, renderPass, nullptr);

		return TReturn::SUCCESS;
	}

	TReturn CreateFramebuffer(uint32_t width, uint32_t height, std::vector<VkImageView>& views, VkRenderPass& renderPass, VkFramebuffer& fbuffer)
	{
		VkFramebufferCreateInfo info
		{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,

			.renderPass = renderPass,

			.attachmentCount = (uint32_t)views.size(),
			.pAttachments = views.data(),

			.width = width,
			.height = height,

			.layers = 1

		};


		VK_CHECK(vkCreateFramebuffer(vk::Device, &info, nullptr, &fbuffer));
		return TReturn::SUCCESS;
	}

	TReturn DestroyFramebuffer(VkFramebuffer& framebuffer)
	{
		vkDestroyFramebuffer(vk::Device, framebuffer, nullptr);

		return TReturn::SUCCESS;
	}



	TReturn GetSupportDetails(SwapchainSupportDetails& details, VkSurfaceKHR& surface)
	{
		details = {};

		// Get Physical Device Surface Capabillities
		VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk::PhysicalDevice, surface, &details.capabillities));

		// Enumerate Surface Formats.
		uint32_t formatsCount;
		VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vk::PhysicalDevice, surface, &formatsCount, nullptr));
		details.formats.resize(formatsCount);
		VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(vk::PhysicalDevice, surface, &formatsCount, details.formats.data()));


		// enumerate presentation modes.
		uint32_t presentModesCount;
		VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(vk::PhysicalDevice, surface, &presentModesCount, nullptr));
		details.presentModes.resize(presentModesCount);
		VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(vk::PhysicalDevice, surface, &presentModesCount, details.presentModes.data()));


		return TReturn::SUCCESS;
	}

	TReturn CreateSwapchain(GLFWwindow* window, VkSurfaceKHR surface, QueueFamily queueFamily, VkFormat format, VkColorSpaceKHR colorspace, uint32_t image_count, VkSwapchainKHR& swapchain)
	{

		-GetSupportDetails(vk::swachainSupportDetails, surface);

		image_count = std::clamp(image_count, vk::swachainSupportDetails.capabillities.minImageCount + 1, vk::swachainSupportDetails.capabillities.maxImageCount);

		bool shouldBeConcurent = (queueFamily.graphics.has_value() && queueFamily.present.has_value()) && (queueFamily.graphics.value() == queueFamily.present.value());
		std::vector<uint32_t> indices;
		if (queueFamily.graphics.has_value()) {
			indices.push_back(queueFamily.graphics.value());
		}
		if (queueFamily.present.has_value()) {
			indices.push_back(queueFamily.present.value());
		}

		auto selectedFormat = SelectFormat(format, colorspace);
		auto selectedPresentMode = SelectPresentMode(VK_PRESENT_MODE_FIFO_KHR); // prefer the best..
		auto selectedExtent = SelectExtent(window);

		VkSwapchainCreateInfoKHR info
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr,
			.flags = 0,
			.surface = surface,

			.minImageCount = image_count,
			.imageFormat = selectedFormat.format,
			.imageColorSpace = selectedFormat.colorSpace,
			.imageExtent = selectedExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.imageSharingMode = shouldBeConcurent ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
			.queueFamilyIndexCount = (uint32_t)indices.size(),
			.pQueueFamilyIndices = indices.data(),
			.preTransform = vk::swachainSupportDetails.capabillities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = selectedPresentMode,
			.clipped = VK_TRUE,

		};

		VK_CHECK(vkCreateSwapchainKHR(vk::Device, &info, nullptr, &swapchain));

		return TReturn::SUCCESS;
	}

	TReturn GetSwapchainImages(VkSwapchainKHR& swapchain, std::vector<VkImage>& images)
	{
		uint32_t imageCount;
		VK_CHECK(vkGetSwapchainImagesKHR(vk::Device, swapchain, &imageCount, nullptr));
		images.resize(imageCount);
		VK_CHECK(vkGetSwapchainImagesKHR(vk::Device, swapchain, &imageCount, images.data()));

		return TReturn::SUCCESS;
	}

	TReturn DestroySwapchain(VkSwapchainKHR swapchain)
	{
		vkDestroySwapchainKHR(vk::Device, swapchain, nullptr);

		return TReturn::SUCCESS;
	}

	TReturn AquireNextImage(VkSemaphore& imageAquiredSemaphore, VkSwapchainKHR swapchain,  uint32_t& currentFrameIndex)
	{
		VkResult res = vkAcquireNextImageKHR(vk::Device, swapchain, UINT64_MAX, imageAquiredSemaphore,VK_NULL_HANDLE, &currentFrameIndex);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
			return TReturn::SUBOPTIMAL;
		}
		else if (res != VK_SUCCESS) {
			// ERROR
			Console::Error("Could Not Aquire Next Image...");
			return TReturn::FAILURE;
		}
		return TReturn::SUCCESS;
	}

	TReturn CreatePipelineLayout( std::vector<VkDescriptorSetLayout>& descriptors, std::vector<VkPushConstantRange>& pushConstants, VkPipelineLayout& layout)
	{
		VkPipelineLayoutCreateInfo layoutCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.setLayoutCount = (uint32_t) descriptors.size(),
			.pSetLayouts = descriptors.data(),
			.pushConstantRangeCount = (uint32_t)pushConstants.size(),
			.pPushConstantRanges = pushConstants.data()
		};

		VK_CHECK(vkCreatePipelineLayout(vk::Device, &layoutCreateInfo, nullptr, &layout));


		return TReturn::SUCCESS;
	}

	TReturn CreateGraphicsPipeline(VkGraphicsPipelineCreateInfo info, VkPipeline& pipeline)
	{
		VK_CHECK(vkCreateGraphicsPipelines(vk::Device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline));

		return TReturn::SUCCESS;
	}

	TReturn DestroyPipeline(VkPipeline& pipeline)
	{
		vkDestroyPipeline(vk::Device, pipeline, nullptr);

		return TReturn::SUCCESS;
	}

	TReturn DestroyPipelineLayout(VkPipelineLayout& layout)
	{
		vkDestroyPipelineLayout(vk::Device, layout, nullptr);

		return TReturn::SUCCESS;
	}

	TReturn CreateCommandPools(QueueFamily family, CommandPoolBlock& block)
	{
		block = { VK_NULL_HANDLE };

		if (family.graphics.has_value())
			- CreateCommandPool(family.graphics.value(), block.Graphics);

		if (family.present.has_value())
			- CreateCommandPool(family.present.value(), block.Present);

		if (family.compute.has_value())
			- CreateCommandPool(family.compute.value(), block.Compute);

		if (family.transfer.has_value())
			- CreateCommandPool(family.transfer.value(), block.Transfer);

		if (family.sparse_binding.has_value())
			- CreateCommandPool(family.sparse_binding.value(), block.SparseBinding);


		return TReturn::SUCCESS;
	}

	TReturn CreateCommandPool(uint32_t index, VkCommandPool& pool) {

		pool = VK_NULL_HANDLE;
		// Create Command Pool
		VkCommandPoolCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = index
		};

		VK_CHECK(vkCreateCommandPool(vk::Device, &info, nullptr, &pool));

		return TReturn::SUCCESS;
	}

	TReturn FreeCommandPoolBlock(CommandPoolBlock& block)
	{
		if (block.Graphics)
			vkDestroyCommandPool(vk::Device, block.Graphics, nullptr);
		if (block.Compute)
			vkDestroyCommandPool(vk::Device, block.Compute, nullptr);
		if (block.Present)
			vkDestroyCommandPool(vk::Device, block.Present, nullptr);
		if (block.Transfer)
			vkDestroyCommandPool(vk::Device, block.Transfer, nullptr);
		if (block.SparseBinding)
			vkDestroyCommandPool(vk::Device, block.SparseBinding, nullptr);
		return TReturn::SUCCESS;
	}

	TReturn AllocateCommandBuffer(VkCommandPool& pool, VkCommandBuffer& cmd)
	{
		cmd = VK_NULL_HANDLE;

		VkCommandBufferAllocateInfo info
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		VK_CHECK(vkAllocateCommandBuffers(vk::Device, &info, &cmd));

		return TReturn::SUCCESS;
	}

	TReturn BeginCommandBufferRecord(VkCommandBuffer& cmd)
	{
		VkCommandBufferBeginInfo begin{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		VK_CHECK(vkBeginCommandBuffer(cmd, &begin));
		return TReturn::SUCCESS;
	}

	TReturn EndCommandBufferRecord(VkCommandBuffer& cmd)
	{
		VK_CHECK(vkEndCommandBuffer(cmd));

		return TReturn::SUCCESS;
	}

	TReturn FreeCommandBuffer(VkCommandPool& pool, VkCommandBuffer& buffer) 
	{

		vkFreeCommandBuffers(vk::Device, pool, 1, &buffer);
		return TReturn::SUCCESS;
	}

	TReturn SubmitCommandBuffer(VkCommandBuffer& cmd, VkQueue& queue,  VkFence& fence) 
	{
		VK_CHECK(vkResetFences(vk::Device, 1, &fence));

		VkSubmitInfo info
		{
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd,
		};

		VK_CHECK(vkQueueSubmit(queue, 1, &info, fence));

		VK_CHECK(vkWaitForFences(vk::Device, 1, &fence, VK_TRUE, UINT64_MAX));
		
		return TReturn::SUCCESS;
	}

	TReturn CreateFenceBlock(FenceBlock& block)
	{
		block = { VK_NULL_HANDLE };

		-CreateFenceSyncOjbect(false, block.Drawing);
		-CreateFenceSyncOjbect(false, block.Presenting);

		return TReturn::SUCCESS;
	}

	TReturn FreeFenceBlock(FenceBlock& block)
	{
		if (block.Drawing != VK_NULL_HANDLE && vkWaitForFences(vk::Device, 1, &block.Drawing, VK_TRUE, UINT64_MAX) == VK_SUCCESS)
			vkDestroyFence(vk::Device, block.Drawing, nullptr);
		if (block.Presenting != VK_NULL_HANDLE && vkWaitForFences(vk::Device, 1, &block.Presenting, VK_TRUE, UINT64_MAX) == VK_SUCCESS)
			vkDestroyFence(vk::Device, block.Presenting, nullptr);

		return TReturn::SUCCESS;
	}

	TReturn CreateFenceSyncOjbect(bool signal, VkFence& fence)
	{
		fence = VK_NULL_HANDLE;

		VkFenceCreateFlags flags = 0;

		if (signal)
			flags |= VK_FENCE_CREATE_SIGNALED_BIT;

		VkFenceCreateInfo fenceInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = flags
		};

		VK_CHECK(vkCreateFence(vk::Device, &fenceInfo, nullptr, &fence));

		return TReturn::SUCCESS;

	}

	TReturn CreateSemaphoreSyncObject(VkSemaphore& semaphore)
	{
		semaphore = VK_NULL_HANDLE;

		VkSemaphoreCreateInfo info
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
		};

		VK_CHECK(vkCreateSemaphore(vk::Device, &info, nullptr, &semaphore));

		return TReturn::SUCCESS;
	}

	TReturn FreeSemaphoreBlock(SemaphoreBlock& block)
	{
		if (block.ImageAvailable != VK_NULL_HANDLE)
			vkDestroySemaphore(vk::Device, block.ImageAvailable, nullptr);

		return TReturn::SUCCESS;
	}

	TReturn CreateSemaphoreBlock(SemaphoreBlock& block)
	{
		block = { VK_NULL_HANDLE };

		-CreateSemaphoreSyncObject(block.ImageAvailable);

		return TReturn::SUCCESS;
	}

	TReturn AllocateImageMemory(VkImage& image, VkDeviceMemory& memory) {
		memory = VK_NULL_HANDLE;

		VkMemoryRequirements requirements{};
		vkGetImageMemoryRequirements(vk::Device, image, &requirements);

		VkMemoryAllocateInfo memoryInfo{};
		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.memoryTypeIndex = requirements.memoryTypeBits;
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VK_CHECK(vkAllocateMemory(vk::Device, &memoryInfo, nullptr, &memory));
		VK_CHECK(vkBindImageMemory(vk::Device, image, memory, 0));

		return TReturn::SUCCESS;
	}

	TReturn FreeImageMemory(VkDeviceMemory& memory)
	{
		vkFreeMemory(vk::Device, memory, nullptr);

		return TReturn::SUCCESS;
	}

	TReturn DestroyImage(VkImage& image) {
		vkDestroyImage(vk::Device, image, nullptr);

		return TReturn::SUCCESS;
	}

	TReturn CreateBuffer(uint32_t size, VkBufferUsageFlags usage, VkBuffer&	buffer)
	{

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.flags = 0;
		info.pNext = nullptr;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.size = size;
		info.usage = usage;

		vkCreateBuffer(vk::Device, &info, nullptr, &buffer);

		return TReturn::SUCCESS;
	}

	TReturn DestroyImageView(VkImageView& view)
	{
		vkDestroyImageView(vk::Device, view, nullptr);
		return TReturn::SUCCESS;
	}

	TReturn CreateImage(uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageUsageFlags usage, VkImageType type, VkImage& image) {
		image = VK_NULL_HANDLE;

		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.format = format;
		info.arrayLayers = 1;
		info.extent.depth = depth;
		info.extent.width = width;
		info.extent.height = height;
		info.imageType = type;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // create the image ready to be written to,
		info.mipLevels = 1;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.usage = usage | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		VK_CHECK(vkCreateImage(vk::Device, &info, nullptr, &image));

		return TReturn::SUCCESS;
	}

	TReturn CreateImageView(VkImage& image, VkFormat format, VkImageViewType type, VkImageView& view) {
		view = VK_NULL_HANDLE;

		VkImageViewCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.format = format;
		info.image = image;
		info.viewType = type;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		info.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;

		VK_CHECK(vkCreateImageView(vk::Device, &info, nullptr, &view));

		return TReturn::SUCCESS;
	}

	TReturn CreateDescriptorSetLayout(DescriptorPoolDesc desc, VkDescriptorSetLayout& layout) {
		layout = VK_NULL_HANDLE;

		assert(desc.total() && "No Bindings Requested.. this is Undefined behavior.");

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		uint32_t bindingCount = -1;

		if (desc.numRequestedSamplers.has_value()) {
			bindings.push_back({
				.binding = ++bindingCount,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.descriptorCount = desc.numRequestedSamplers.value(),
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				});
		}

		if (desc.numRequestedCombinedSamplers.has_value()) {
			bindings.push_back({
				.binding = ++bindingCount,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = desc.numRequestedCombinedSamplers.value(),
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				});
		}

		if (desc.numRequestedUniformBuffers.has_value()) {
			bindings.push_back({
				.binding = ++bindingCount,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount = desc.numRequestedUniformBuffers.value(),
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				});
		}

		if (desc.numRequestedStorageBuffers.has_value()) {
			bindings.push_back({
				.binding = ++bindingCount,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.descriptorCount = desc.numRequestedStorageBuffers.value(),
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				});
		}


		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.flags = 0;
		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		VK_CHECK(vkCreateDescriptorSetLayout(vk::Device, &layoutInfo, nullptr, &layout));

		return TReturn::SUCCESS;
	}

	TReturn AllocateDescriptorSet(VkDescriptorPool& pool, VkDescriptorSetLayout& layout, VkDescriptorSet& descriptorSet)
	{
		descriptorSet = nullptr;

		VkDescriptorSetAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		info.pNext = nullptr;
		info.descriptorPool = pool;
		info.descriptorSetCount = 1;
		info.pSetLayouts = &layout;

		VK_CHECK(vkAllocateDescriptorSets(vk::Device, &info, &descriptorSet));

		return TReturn::SUCCESS;

	}

	TReturn CreateDescriptorPool(DescriptorPoolDesc desc, VkDescriptorPool& pool)
	{
		pool = VK_NULL_HANDLE;

		std::vector<VkDescriptorPoolSize> descriptorPoolDesc;
		if (desc.numRequestedSamplers.has_value()) {
			descriptorPoolDesc.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER, desc.numRequestedSamplers.value() });
		}
		if (desc.numRequestedCombinedSamplers.has_value()) {
			descriptorPoolDesc.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, desc.numRequestedCombinedSamplers.value() });
		}
		if (desc.numRequestedUniformBuffers.has_value()) {
			descriptorPoolDesc.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, desc.numRequestedUniformBuffers.value() });
		}
		if (desc.numRequestedStorageBuffers.has_value()) {
			descriptorPoolDesc.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, desc.numRequestedStorageBuffers.value() });
		}

		VkDescriptorPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.maxSets = 8;
		info.poolSizeCount = descriptorPoolDesc.size();
		info.pPoolSizes = descriptorPoolDesc.data();

		VK_CHECK(vkCreateDescriptorPool(vk::Device, &info, nullptr, &pool))

			return TReturn::SUCCESS;

	}


	VkSurfaceFormatKHR SelectFormat(VkFormat format, VkColorSpaceKHR colorSpace)
	{
		auto formats = vk::swachainSupportDetails.formats;

		for (auto f = formats.begin(); f != formats.end(); f++) {
			if (f->format == format && f->colorSpace == colorSpace) {
				return *f;
			}
		}


		return formats[0];
	}

	VkPresentModeKHR SelectPresentMode(VkPresentModeKHR presentMode)
	{
		auto modes = vk::swachainSupportDetails.presentModes;

		for (auto m = modes.begin(); m != modes.end(); m++)
		{
			if (*m == presentMode)
				return *m;
		}

		return modes[0];
	}

	VkExtent2D SelectExtent(GLFWwindow* window)
	{
		auto capabillities = vk::swachainSupportDetails.capabillities;

		VkExtent2D extent;

		if (capabillities.currentExtent.width != UINT32_MAX)
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

	TReturn CreateShaderModule(std::vector<unsigned int> data, VkShaderModule& shaderModule)
	{
		VkShaderModuleCreateInfo create{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = data.size() * sizeof(unsigned int),
		.pCode = data.data(),
		};

		VK_CHECK(vkCreateShaderModule(vk::Device, &create, nullptr, &shaderModule));
		
		return TReturn::SUCCESS;
	}

	TReturn DestroyShaderModule(VkShaderModule& shaderModule)
	{
		vkDestroyShaderModule(vk::Device, shaderModule, nullptr);

		return TReturn::SUCCESS;
	}

}
