#pragma once

#include "graphics/vulkan_api.h"

enum class TextureFormat {
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

class Texture2D {

public:
	Texture2D(VkDevice device, uint32_t width, uint32_t height, TextureFormat format);
	Texture2D(VkDevice device, uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, TextureType type);

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