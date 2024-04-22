#pragma once


#include "Graphics/Rendering/Utils/RenderPipelineFactory.h"
#include "Graphics/Rendering/Shaders/ShaderGraph.h"

#include <filesystem>
#include <debug/Console.h>

namespace RenderPipelines {

	class Basic2D : public RenderPipeline {

		struct Vertex {
			struct {
				float x, y, z = 0;
			}Position;
			struct {
				float r, g, b, a = 0;
			}Color;
		};

		ShaderGraph* vertex;
		ShaderGraph* fragment;

		void CreateShaders() {

			// Crate Shader Module and Compile to SPIR-V From File
			auto baseDir = std::filesystem::current_path() / "Shaders";
			auto vertexFilepath = baseDir / "vertex.hlsl";

			// describe what will happen in the vertex stage..
			vertex = new ShaderGraph(vertexFilepath.generic_string(), VK_SHADER_STAGE_VERTEX_BIT);
			vertex->AddInput(0, ShaderVarType::_VEC3_, "Position", 0);
			vertex->AddInput(1, ShaderVarType::_VEC3_, "Color", 0);

			vertex->AddOutput(0, ShaderVarType::_VEC4_, "FragColor");

			vertex->AddMain("\tgl_Position = vec4(Position, 1.0);\n\tFragColor = vec4(Color, 1.0);\n");

			-vertex->Compile();
			

			auto fragmentFilepath = baseDir / "fragment.hlsl";

			fragment = new ShaderGraph(fragmentFilepath.generic_string(), VK_SHADER_STAGE_FRAGMENT_BIT);
			fragment->AddInput(0, ShaderVarType::_VEC4_, "FragColor", 0);
			fragment->AddOutput(0, ShaderVarType::_VEC4_, "OutputColor");

			fragment->AddMain("\tOutputColor = FragColor;\n");

			-fragment->Compile();
			

		}

		virtual void CreateDescriptorPool() override {
			VulkanAPI::DescriptorPoolDesc desc{};
			desc.numRequestedSamplers = 1;

			if (desc.total() <= 0)
				return;

			 -VulkanAPI::CreateDescriptorPool(desc, descriptorPool);
			 -VulkanAPI::CreateDescriptorSetLayout(desc, descriptorSetLayout);
			 -VulkanAPI::AllocateDescriptorSet(descriptorPool, descriptorSetLayout, descriptorSet);
		}

		virtual void CreateRenderPass() override {
			std::vector<VkAttachmentDescription> renderPassAttachments
			{
				// Pass 0
				{
					.format = VK_FORMAT_B8G8R8A8_UNORM,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				}
			};

			VulkanAPI::CreateRenderPass(renderPassAttachments, renderPass);
			
		}

		virtual void CreatePipeline() override
		{
			

			std::vector<VkPushConstantRange> pushConstants{};
			std::vector<VkDescriptorSetLayout> descriptors{};

			-VulkanAPI::CreatePipelineLayout(descriptors, pushConstants, layout);

			VkPipelineRasterizationStateCreateInfo RS{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_TRUE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f
			};

			// Describe Shader Stages

			std::vector<VkVertexInputBindingDescription> bindings{
				{.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }
			};

			CreateShaders();

			std::vector<VkPipelineShaderStageCreateInfo> stages =
			{
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.stage = VK_SHADER_STAGE_VERTEX_BIT,
					.module = vertex->Get(),
					.pName = "main",

				},
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
					.module = fragment->Get(),
					.pName = "main"
				}
			};


			auto attribs = vertex->GetAttributes();



			VkPipelineVertexInputStateCreateInfo VI 
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,

				.vertexBindingDescriptionCount = (uint32_t)bindings.size(),
				.pVertexBindingDescriptions = bindings.data(),
				.vertexAttributeDescriptionCount = (uint32_t)attribs.size(),
				.pVertexAttributeDescriptions = attribs.data()
			};


			VkPipelineInputAssemblyStateCreateInfo IA
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.primitiveRestartEnable = VK_FALSE,
			};

			std::vector<VkViewport> viewports
			{
				{
					.x = 0, .y = 0, .width = static_cast<float>(InternalResolution.width), .height = static_cast<float>(InternalResolution.height), .minDepth = 0, .maxDepth = 1
				}
			};

			std::vector<VkRect2D> scissors
			{
				{
					.offset = { 0, 0 },
					.extent = { InternalResolution.width, InternalResolution.height }
				}
			};

			VkPipelineViewportStateCreateInfo VS
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.viewportCount = (uint32_t)viewports.size(),
				.pViewports = viewports.data(),
				.scissorCount = (uint32_t)scissors.size(),
				.pScissors = scissors.data()
			};


			VkPipelineMultisampleStateCreateInfo MSS
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable = VK_FALSE,
				.minSampleShading = 0,
				.pSampleMask = nullptr,
				.alphaToCoverageEnable = VK_FALSE,
				.alphaToOneEnable = VK_FALSE,
			};


			std::vector<VkPipelineColorBlendAttachmentState> blendAttachments
			{
				{
					.blendEnable = VK_TRUE,

					.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
					.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					.colorBlendOp = VK_BLEND_OP_ADD,

					.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
					.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
					.alphaBlendOp = VK_BLEND_OP_ADD,

					.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
				}
			};

			VkPipelineColorBlendStateCreateInfo BLEND
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,

				.logicOpEnable = VK_FALSE,
				.logicOp = VK_LOGIC_OP_COPY,

				.attachmentCount = (uint32_t)blendAttachments.size(),
				.pAttachments = blendAttachments.data(),

			};

			VkGraphicsPipelineCreateInfo info{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stageCount = (uint32_t)stages.size(),
				.pStages = stages.data(),
				.pVertexInputState = &VI,
				.pInputAssemblyState = &IA,
				.pViewportState = &VS,
				.pRasterizationState = &RS,
				.pMultisampleState = &MSS,
				.pColorBlendState = &BLEND,
				.layout = layout,
				.renderPass = renderPass,
				.subpass = 0
			};

			-VulkanAPI::CreateGraphicsPipeline(info, pipeline);
		}

		virtual void OnDestroyPipeline() override {
			
			delete vertex;
			delete fragment;

		};
	};

	class Screen : public RenderPipeline {
		struct Vertex {
			float position[3];
			float uv[2];
		};

		ShaderGraph* vertex;
		ShaderGraph* fragment;

		void CreateShaders() {

			// Crate Shader Module and Compile to SPIR-V From File
			auto baseDir = std::filesystem::current_path() / "Shaders/screen/";
			auto vertexFilepath = baseDir / "vertex.hlsl";

			// describe what will happen in the vertex stage..
			vertex = new ShaderGraph(vertexFilepath.generic_string(), VK_SHADER_STAGE_VERTEX_BIT);
			vertex->AddInput(0, ShaderVarType::_VEC3_, "Position", 0);

			vertex->AddOutput(0, ShaderVarType::_VEC2_, "FTexCoord");
			
			vertex->AddMain("\tgl_Position = vec4(Position, 1.0);\n\tFTexCoord = TexCoord;\n");

			-vertex->Compile();
			

			auto fragmentFilepath = baseDir / "fragment.hlsl";

			fragment = new ShaderGraph(fragmentFilepath.generic_string(), VK_SHADER_STAGE_FRAGMENT_BIT);
			fragment->AddInput(0, ShaderVarType::_VEC2_, "FTexCoord", 0);
			fragment->AddOutput(0, ShaderVarType::_VEC4_, "OutputColor");
			fragment->AddTexture2D("");
			fragment->AddMain("\tOutputColor = FragColor;\n");

			-fragment->Compile();

		}
		

		virtual void CreateDescriptorPool() override {
			VulkanAPI::DescriptorPoolDesc desc{};
			desc.numRequestedSamplers = 1;

			if (desc.total() <= 0)
				return;

			-VulkanAPI::CreateDescriptorPool(desc, descriptorPool);
			-VulkanAPI::CreateDescriptorSetLayout(desc, descriptorSetLayout);
			-VulkanAPI::AllocateDescriptorSet(descriptorPool, descriptorSetLayout, descriptorSet);
		}

		virtual void CreateRenderPass() override {
			std::vector<VkAttachmentDescription> renderPassAttachments
			{
				// Pass 0
				{
					.format = VK_FORMAT_B8G8R8A8_UNORM,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				}
			};

			VulkanAPI::CreateRenderPass(renderPassAttachments, renderPass);

		}

		virtual void CreatePipeline() override
		{


			std::vector<VkPushConstantRange> pushConstants{};
			std::vector<VkDescriptorSetLayout> descriptors{};

			-VulkanAPI::CreatePipelineLayout(descriptors, pushConstants, layout);

			VkPipelineRasterizationStateCreateInfo RS{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.pNext = 0,
			.flags = 0,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_TRUE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f
			};

			// Describe Shader Stages

			std::vector<VkVertexInputBindingDescription> bindings{
				{.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }
			};

			CreateShaders();

			std::vector<VkPipelineShaderStageCreateInfo> stages =
			{
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.stage = VK_SHADER_STAGE_VERTEX_BIT,
					.module = vertex->Get(),
					.pName = "main",

				},
				{
					.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
					.pNext = nullptr,
					.flags = 0,
					.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
					.module = fragment->Get(),
					.pName = "main"
				}
			};


			auto attribs = vertex->GetAttributes();



			VkPipelineVertexInputStateCreateInfo VI
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,

				.vertexBindingDescriptionCount = (uint32_t)bindings.size(),
				.pVertexBindingDescriptions = bindings.data(),
				.vertexAttributeDescriptionCount = (uint32_t)attribs.size(),
				.pVertexAttributeDescriptions = attribs.data()
			};


			VkPipelineInputAssemblyStateCreateInfo IA
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.primitiveRestartEnable = VK_FALSE,
			};

			std::vector<VkViewport> viewports
			{
				{
					.x = 0, .y = 0, .width = static_cast<float>(InternalResolution.width), .height = static_cast<float>(InternalResolution.height), .minDepth = 0, .maxDepth = 1
				}
			};

			std::vector<VkRect2D> scissors
			{
				{
					.offset = { 0, 0 },
					.extent = { InternalResolution.width, InternalResolution.height }
				}
			};

			VkPipelineViewportStateCreateInfo VS
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.viewportCount = (uint32_t)viewports.size(),
				.pViewports = viewports.data(),
				.scissorCount = (uint32_t)scissors.size(),
				.pScissors = scissors.data()
			};


			VkPipelineMultisampleStateCreateInfo MSS
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable = VK_FALSE,
				.minSampleShading = 0,
				.pSampleMask = nullptr,
				.alphaToCoverageEnable = VK_FALSE,
				.alphaToOneEnable = VK_FALSE,
			};


			std::vector<VkPipelineColorBlendAttachmentState> blendAttachments
			{
				{
					.blendEnable = VK_TRUE,

					.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
					.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
					.colorBlendOp = VK_BLEND_OP_ADD,

					.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
					.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
					.alphaBlendOp = VK_BLEND_OP_ADD,

					.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
				}
			};

			VkPipelineColorBlendStateCreateInfo BLEND
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,

				.logicOpEnable = VK_FALSE,
				.logicOp = VK_LOGIC_OP_COPY,

				.attachmentCount = (uint32_t)blendAttachments.size(),
				.pAttachments = blendAttachments.data(),

			};

			VkGraphicsPipelineCreateInfo info{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
				.flags = 0,
				.stageCount = (uint32_t)stages.size(),
				.pStages = stages.data(),
				.pVertexInputState = &VI,
				.pInputAssemblyState = &IA,
				.pViewportState = &VS,
				.pRasterizationState = &RS,
				.pMultisampleState = &MSS,
				.pColorBlendState = &BLEND,
				.layout = layout,
				.renderPass = renderPass,
				.subpass = 0
			};

			-VulkanAPI::CreateGraphicsPipeline(info, pipeline);
		}

		virtual void OnDestroyPipeline() override {

			delete vertex;
			delete fragment;

		};


	};
}
