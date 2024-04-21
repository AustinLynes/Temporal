#pragma once

#include "graphics/vulkan_api.h"
#include "../Textures/Texture2D.h"

class TextureFactory {
public:
	static Texture2D* CreateTexture2D(VkDevice device, uint32_t width, uint32_t height, TextureFormat format);
	static Texture2D* CreateTexture2D(VkDevice device, uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, TextureType type);
};