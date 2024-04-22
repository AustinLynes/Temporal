#pragma once

#include "graphics/vulkan_api.h"
#include "../Textures/Texture2D.h"

class TextureFactory {
public:
	static Texture2D* CreateFramebufferTexture2D(int32_t width, uint32_t height, TextureFormat format);
	static Texture2D* CreateFramebufferTexture2D(uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, TextureType type);

	static Texture2D* CreateSampledTexture2D(uint32_t width, uint32_t height, TextureFormat format);
	static Texture2D* CreateSampledTexture2D(uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, TextureType type);

	static void DestroyTexture2D(Texture2D* texture2D);

};