#include "Texture2D.h"

typedef unsigned char Byte;




VkFormat ToVkFormat(TextureFormat f) {
	switch (f)
	{
	case TextureFormat::BGRA8_UNORM:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case TextureFormat::ARGB8_UNORM:
	case TextureFormat::RGBA8_UNORM:
		return VK_FORMAT_R8G8B8A8_UNORM;

	case TextureFormat::ARGB32_SFLOAT:
	case TextureFormat::RGBA32_SFLOAT:
	case TextureFormat::BGRA32_SFLOAT:
	case TextureFormat::GBRA32_SFLOAT:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:
		break;
	}
}

VkImageType ToVkImageType(TextureType t) {
	switch (t)
	{
	case TextureType::Texture1D:
		return VK_IMAGE_TYPE_1D;
	case TextureType::Texture2D:
		return VK_IMAGE_TYPE_2D;
	case TextureType::Texture3D:
		return VK_IMAGE_TYPE_3D;
	default:
		return {};
	}
}

VkImageViewType ToVkImageViewType(TextureType t) {
	switch (t)
	{
	case TextureType::Texture1D:
		return VK_IMAGE_VIEW_TYPE_1D;
	case TextureType::Texture2D:
		return VK_IMAGE_VIEW_TYPE_2D;
	case TextureType::Texture3D:
		return VK_IMAGE_VIEW_TYPE_3D;
	case TextureType::CubeMap:
		return VK_IMAGE_VIEW_TYPE_CUBE;
	case TextureType::TextureArray1D:
		return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	case TextureType::TextureArray2D:
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case TextureType::CubeMapArray:
		return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
	default:
		break;
	}
}

VkImageUsageFlags ToVkImageUsage(TextureUsage usage) {
	switch (usage)
	{
	case TextureUsage::FrambufferAttachment:
		return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	case TextureUsage::SampleAttachment:
		return VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
}


Texture2D::Texture2D(uint32_t width, uint32_t height, TextureFormat format, TextureUsage usage)
{
	VulkanAPI::CreateImage( width, height, 1, ToVkFormat(format), ToVkImageUsage(usage), ToVkImageType(TextureType::Texture2D), image);
	VulkanAPI::AllocateImageMemory(image, memory);
	VulkanAPI::CreateImageView(image, ToVkFormat(format), ToVkImageViewType(TextureType::Texture2D), view);
}

Texture2D::Texture2D(uint32_t width, uint32_t height, uint32_t depth, TextureFormat format,  TextureUsage usage, TextureType type)
{
	VulkanAPI::CreateImage(width, height, depth, ToVkFormat(format), ToVkImageUsage(usage), ToVkImageType(type), image);
	VulkanAPI::AllocateImageMemory(image, memory);
	VulkanAPI::CreateImageView(image, ToVkFormat(format), ToVkImageViewType(type), view);
}

Texture2D::~Texture2D()
{
	VulkanAPI::DestroyImageView(view);
	VulkanAPI::FreeImageMemory(memory);
	VulkanAPI::DestroyImage(image);

}


int Texture2D::Load(void* data)
{
	return 0;
}

const VkImageView& Texture2D::View()
{
	return view;
}

const VkImage& Texture2D::Image()
{
	return image;
}

const VkSampler& Texture2D::Sampler()
{
	return sampler;
}
