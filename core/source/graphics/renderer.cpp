#include "renderer.h"


#define VOLK_IMPLEMENTATION
#include <volk.h>
#include "Shader.h"
#include <filesystem>



namespace vk
{
	VkInstance Instance = { VK_NULL_HANDLE };
	VkPhysicalDevice PhysicalDevice = { VK_NULL_HANDLE };
	VkDevice Device = { VK_NULL_HANDLE };

	QueueHandleBlock Queues;
	QueueFamilyIndices QueueFamily;

	CommandPoolBlock CommandPools;
	FenceBlock Fences;

	VkSurfaceKHR Surface;

	// temporary members for single frame drawing test
	VkPipeline pipeline;
	VkRenderPass renderPass;
	VkFramebuffer framebuffer;
	VkImage image;
	VkImageView imageView;


	std::vector<std::string> layers = {};
	std::vector<std::string> extensions = {};

}// GLOBALS


// Util Function used for filtering sets of string
std::unordered_set<std::string> filter(std::vector<std::string> available, std::vector<std::string> requested) {
	std::sort(available.begin(), available.end());
	std::sort(requested.begin(), requested.end());
	std::vector<std::string> result;
	std::set_intersection(available.begin(), available.end(), requested.begin(), requested.end(), std::back_inserter(result));

	return std::unordered_set<std::string>(result.begin(), result.end());
}

Renderer::Renderer()
{

}

Renderer::~Renderer()
{

}

bool Renderer::Initilize(GLFWwindow* window)
{
	volkInitialize();

	GetRequiredInfo();
	// Create Instance 
	vk::Instance = CreateInstance();
	// Create Surface
	vk::Surface = CreateSurface(window);
	// Select A Physical Device
	vk::PhysicalDevice = GetPhysicalDevice();
	// Create Device
	vk::QueueFamily = ReserveQueueFamily();
	vk::Device = CreateDevice();
	vk::Queues = AquireQueueHandles();

	vk::Fences = CreateFenceBlock();

	// Create Required Command Pools for required Queues
	vk::CommandPools = CreateCommandPools(vk::QueueFamily);

	// temporary 
	{
		vk::pipeline = CreatePipeline(vk::Device, PipelineType::Graphics);


		RenderFrame();
		PresentFrame();
	}

	return vk::Instance != VK_NULL_HANDLE && vk::PhysicalDevice != VK_NULL_HANDLE && vk::Device != VK_NULL_HANDLE;
}

// allocate a command buffer to record commands to.
// begin recording process on allocated command buffer
// ... 
// record commands
// ...
// end recording process on allocated command buffer.
// submit command buffer 
void Renderer::RenderFrame() {

	VkCommandBuffer cmd{ VK_NULL_HANDLE };
	{
		VkCommandBufferAllocateInfo info
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.pNext = nullptr,
			.commandPool = vk::CommandPools.Graphics,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		VK_CHECK(vkAllocateCommandBuffers(vk::Device, &info, &cmd));
	}

	VkCommandBufferBeginInfo begin{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT
	};
	vkBeginCommandBuffer(cmd, &begin);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vk::pipeline);

	vkEndCommandBuffer(cmd);

	VkSubmitInfo submit{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
	};


	VK_CHECK(vkResetFences(vk::Device, 1, &vk::Fences.Drawing));

	VK_CHECK(vkQueueSubmit(vk::Queues.Graphics, 1, &submit, vk::Fences.Drawing));


}

void Renderer::PresentFrame()
{
	vkWaitForFences(vk::Device, 1, &vk::Fences.Drawing, VK_FALSE, UINT64_MAX);
}


void Renderer::Cleanup()
{
	// perform cleanup in reverse order
	vkDestroyDevice(vk::Device, nullptr);
	vkDestroySurfaceKHR(vk::Instance, vk::Surface, nullptr);
	vkDestroyInstance(vk::Instance, nullptr);
}

QueueHandleBlock Renderer::AquireQueueHandles() {
	QueueHandleBlock block{ VK_NULL_HANDLE };

	if (vk::QueueFamily.graphics.has_value())
		block.Graphics = AquireQueueHandle(CommandType::Graphics);
	if (vk::QueueFamily.present.has_value())
		block.Present = AquireQueueHandle(CommandType::Present);
	if (vk::QueueFamily.compute.has_value())
		block.Compute = AquireQueueHandle(CommandType::Compute);
	if (vk::QueueFamily.transfer.has_value())
		block.Transfer = AquireQueueHandle(CommandType::Transfer);
	if (vk::QueueFamily.sparse_binding.has_value())
		block.SparseBinding = AquireQueueHandle(CommandType::SparseBinding);

	return block;
}

VkQueue Renderer::AquireQueueHandle(CommandType type) {
	if (vk::Instance == VK_NULL_HANDLE || vk::PhysicalDevice == VK_NULL_HANDLE || vk::Device == VK_NULL_HANDLE)
		return VK_NULL_HANDLE;

	VkQueue queue{ VK_NULL_HANDLE };

	switch (type)
	{
	case CommandType::Graphics:
		vkGetDeviceQueue(vk::Device, vk::QueueFamily.graphics.value(), 0, &queue);
		break;
	case CommandType::Present:
		vkGetDeviceQueue(vk::Device, vk::QueueFamily.present.value(), 0, &queue);
		break;
	case CommandType::Compute:
		vkGetDeviceQueue(vk::Device, vk::QueueFamily.compute.value(), 0, &queue);
		break;
	case CommandType::Transfer:
		vkGetDeviceQueue(vk::Device, vk::QueueFamily.transfer.value(), 0, &queue);
		break;
	case CommandType::SparseBinding:
		vkGetDeviceQueue(vk::Device, vk::QueueFamily.sparse_binding.value(), 0, &queue);
		break;
	}

	return queue;
}

VkSurfaceKHR Renderer::CreateSurface(GLFWwindow* window)
{
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	VkWin32SurfaceCreateInfoKHR info{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.hinstance = GetModuleHandle(NULL),
		.hwnd = (HWND)glfwGetWin32Window(window),
	};

	VK_CHECK(vkCreateWin32SurfaceKHR(vk::Instance, &info, nullptr, &surface));

	return surface;
}

CommandPoolBlock Renderer::CreateCommandPools(QueueFamilyIndices family)
{
	CommandPoolBlock block{ VK_NULL_HANDLE };

	if (family.graphics.has_value())
		block.Graphics = CreateCommandPool(family.graphics.value());

	if (family.present.has_value())
		block.Present = CreateCommandPool(family.present.value());

	if (family.compute.has_value())
		block.Compute = CreateCommandPool(family.compute.value());

	if (family.transfer.has_value())
		block.Transfer = CreateCommandPool(family.transfer.value());

	if (family.sparse_binding.has_value())
		block.SparseBinding = CreateCommandPool(family.sparse_binding.value());


	return block;
}

VkCommandPool Renderer::CreateCommandPool(uint32_t index) {

	VkCommandPool pool = VK_NULL_HANDLE;
	// Create Command Pool
	VkCommandPoolCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = index
	};

	VK_CHECK(vkCreateCommandPool(vk::Device, &info, nullptr, &pool));

	return pool;
}

FenceBlock Renderer::CreateFenceBlock()
{
	FenceBlock block;

	block.Drawing = CreateFence();
	block.Presenting = CreateFence();

	return block;
}

VkFence Renderer::CreateFence()
{
	VkFence fence = VK_NULL_HANDLE;

	VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	VK_CHECK(vkCreateFence(vk::Device, &fenceInfo, nullptr, &fence));

	return fence;

}

VkPipeline Renderer::CreatePipeline(VkDevice device, PipelineType type)
{
	VkPipeline pipeline = VK_NULL_HANDLE;
	switch (type) {
	case PipelineType::Graphics:
	{
		
		// Crate Shader Module and Compile to SPIR-V From File
		auto baseDir = std::filesystem::current_path() / "Shaders";
		auto vertexFilepath = baseDir / "vertex.hlsl";

		Shader vertexShader(device, vertexFilepath.generic_string(), VK_SHADER_STAGE_VERTEX_BIT);
		vertexShader.AddInput(0, ShaderVarType::_VEC3_, "Position");
		vertexShader.AddInput(1, ShaderVarType::_VEC3_, "Color");

		vertexShader.AddOutput(ShaderVarType::_VEC4_, "FragColor");
		
		vertexShader.Compile();

		std::vector<VkPipelineShaderStageCreateInfo> stages=
		{
			{
				.sType= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext =nullptr, 
				.flags = 0,
				.module = vertexShader.Get(),
				.pName=""

			}
		};

		/// TODO! Create a simple graphics pipeline that can render something to screen
		VkGraphicsPipelineCreateInfo info{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stageCount = (uint32_t)stages.size(),
			.pStages = stages.data(),

			
		};

		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);

	}
	break;
	case PipelineType::Compute: {
		VkComputePipelineCreateInfo info{};

		/// TODO! Create a Compute Pipeline
		vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);
	}
	break;
	}


	return pipeline;
}


void Renderer::GetRequiredInfo()
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
		vk::extensions.push_back(glfwRequiredExtensions[i]);
	}
}

VkInstance Renderer::CreateInstance() {

	VkInstance inst;
	// enumerate layers
	// then enumerate properties
	uint32_t layerCount = -1;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
	std::vector<VkLayerProperties> found_layers(layerCount);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, found_layers.data()));

	std::vector<std::string> available_layers;

	std::transform(found_layers.begin(), found_layers.end(), std::back_inserter(available_layers), [](const VkLayerProperties& l) {return l.layerName; });

	auto layers = filter(available_layers, vk::layers);

	std::unordered_set<std::string> avialable_extensions;

	for (auto i = layers.begin(); i != layers.end(); i++)
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

	for (auto i = vk::extensions.begin(); i != vk::extensions.end(); i++) {
		avialable_extensions.insert(i->c_str());
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
	VK_CHECK(vkCreateInstance(&info, nullptr, &inst));

	volkLoadInstance(inst);

	return inst;
}

VkPhysicalDevice Renderer::GetPhysicalDevice() {

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

	VkPhysicalDevice device = scores[0].device;

	std::cout << "Selected Device: " << scores[0].props.deviceName << std::endl;

	return device;
}

QueueFamilyIndices Renderer::ReserveQueueFamily() {
	if (vk::Instance == VK_NULL_HANDLE || vk::PhysicalDevice == VK_NULL_HANDLE || vk::Surface == VK_NULL_HANDLE)
		return { -1, -1 }; // INVALID INDICES

	QueueFamilyIndices family;

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
		vkGetPhysicalDeviceSurfaceSupportKHR(vk::PhysicalDevice, i, vk::Surface, &presentSupport);

		if (presentSupport) {
			family.present = i;
		}


		i++;

	}

	std::cout << "Queue Family:\n"
		<< "\t[" << (family.graphics.has_value() ? "X" : " ") << "]" << " Graphics\n" << "\t\t id: " << (family.graphics.has_value() ? std::to_string(family.graphics.value()) : " ") << "\n"
		<< "\t[" << (family.present.has_value() ? "X" : " ") << "]" << " Present\n" << "\t\t id: " << (family.present.has_value() ? std::to_string(family.present.value()) : " ") << "\n"
		<< "\t[" << (family.compute.has_value() ? "X" : " ") << "]" << " Compute\n" << "\t\t id: " << (family.compute.has_value() ? std::to_string(family.compute.value()) : " ") << "\n"
		<< "\t[" << (family.transfer.has_value() ? "X" : " ") << "]" << " Transfer\n" << "\t\t id: " << (family.transfer.has_value() ? std::to_string(family.transfer.value()) : " ") << "\n"
		<< "\t[" << (family.sparse_binding.has_value() ? "X" : " ") << "]" << " Sparse Binding\n" << "\t\t id: " << (family.sparse_binding.has_value() ? std::to_string(family.sparse_binding.value()) : " ") << "\n"
		<< "\n";

	return family;
}

VkDevice Renderer::CreateDevice() {
	if (vk::Instance == VK_NULL_HANDLE || vk::PhysicalDevice == VK_NULL_HANDLE)
		return VK_NULL_HANDLE;

	std::vector<const char*> extensions;
	std::transform(vk::extensions.begin(), vk::extensions.end(), std::back_inserter(extensions), [](const std::string& str) {return str.c_str(); });

	std::vector<const char*> layers;
	std::transform(vk::layers.begin(), vk::layers.end(), std::back_inserter(layers), [](const std::string& str) {return str.c_str(); });

	VkDevice device;

	float priority = 1;

	auto queues = vk::QueueFamily.GetCreateInfo();

	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueCreateInfoCount = (uint32_t)queues.size(),
		.pQueueCreateInfos = queues.data(),
		.pEnabledFeatures = &deviceFeatures,
	};

	vkCreateDevice(vk::PhysicalDevice, &info, nullptr, &device);

	volkLoadDevice(device);

	return device;

}

