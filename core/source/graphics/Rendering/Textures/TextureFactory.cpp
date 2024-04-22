#include "TextureFactory.h"


Texture2D* TextureFactory::CreateSampledTexture2D(uint32_t width, uint32_t height, TextureFormat format)
{
    return new Texture2D(width, height,  format, TextureUsage::SampleAttachment);
}

Texture2D* TextureFactory::CreateSampledTexture2D(uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, TextureType type)
{
    return new Texture2D(width, height, depth, format, TextureUsage::SampleAttachment, type);
}

Texture2D* TextureFactory::CreateFramebufferTexture2D(int32_t width, uint32_t height, TextureFormat format)
{
    return new Texture2D(width, height,  format, TextureUsage::SampleAttachment);
}

Texture2D* TextureFactory::CreateFramebufferTexture2D(uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, TextureType type)
{
    return new Texture2D(width, height, depth, format, TextureUsage::FrambufferAttachment, type);
}

void TextureFactory::DestroyTexture2D(Texture2D* texture2D)
{
    delete texture2D;
}
