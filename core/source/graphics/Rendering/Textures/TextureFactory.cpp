#include "TextureFactory.h"


Texture2D* TextureFactory::CreateTexture2D(VkDevice device, uint32_t width, uint32_t height, TextureFormat format)
{
    return new Texture2D(device, width, height, format);
}

Texture2D* TextureFactory::CreateTexture2D(VkDevice device, uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, TextureType type)
{
    return new Texture2D(device, width, height, depth, format, type);
}
