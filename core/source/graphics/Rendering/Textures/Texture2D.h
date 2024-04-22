#pragma once

#include "graphics/vulkan_api.h"

enum class TextureFormat {
	BGRA8_UNORM,
	ARGB8_UNORM,
	GBRA8_UNORM,
	RGBA8_UNORM,

	ARGB32_SFLOAT,
	RGBA32_SFLOAT,
	BGRA32_SFLOAT,
	GBRA32_SFLOAT,
};
enum class TextureType {
	Texture1D,
	Texture2D,
	Texture3D,
	CubeMap,
	TextureArray1D,
	TextureArray2D,
	CubeMapArray,
};

enum class TextureUsage {
	FrambufferAttachment,
	SampleAttachment
};

class Texture2D {

public:
	Texture2D(uint32_t width, uint32_t height, TextureFormat format, TextureUsage usage);
	Texture2D(uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, TextureUsage usage, TextureType type);
	~Texture2D();
	
	int Load(void* data);

	const VkImageView& View();
	const VkImage& Image();
	const VkSampler& Sampler();

private:
	VkImage image;
	VkImageView view;
	VkDeviceMemory memory;
	VkSampler sampler;
};