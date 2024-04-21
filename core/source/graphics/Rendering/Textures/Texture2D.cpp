#include "Texture2D.h"

typedef unsigned char Byte;


#if 0 // DUMMY DATA
struct {
	const uint32_t width;
	const uint32_t height;
	const TextureFormat format = TextureFormat::ARGB32_SFLOAT;
	// ABGR
	const std::vector<Byte> data = { 
		 0xFF, 0x00, 0xFF, 0x00 , 0xFF, 0x00, 0xFF, 0x00 ,  0xFF, 0x00, 0x00, 0xFF , 0xFF, 0x00, 0xFF, 0x00 ,
		 0xFF, 0x00, 0xFF, 0x00 , 0xFF, 0x00, 0xFF, 0x00 ,  0xFF, 0x00, 0x00, 0xFF , 0xFF, 0x00, 0xFF, 0x00 ,
		 0xFF, 0xFF, 0x00, 0x00 , 0xFF, 0xFF, 0x00, 0x00 ,  0xFF, 0xFF, 0x00, 0xFF , 0xFF, 0xFF, 0xFF, 0x00 ,
		 0xFF, 0xFF, 0x00, 0x00 , 0xFF, 0xFF, 0x00, 0x00 ,  0xFF, 0xFF, 0x00, 0xFF , 0xFF, 0xFF, 0xFF, 0x00 ,
	};
}DUMMY_IMAGE;
#endif

VkFormat ToVkFormat(TextureFormat f) {
	switch (f)
	{
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


Texture2D::Texture2D(VkDevice device, uint32_t width, uint32_t height, TextureFormat format)
{
	image = VulkanAPI::CreateImage(device, width, height, 1, ToVkFormat(format), ToVkImageType(TextureType::Texture1D));
	memory = VulkanAPI::AllocateImageMemory(device, image);
	view = VulkanAPI::CreateImageView(device, image, ToVkFormat(format), ToVkImageViewType(TextureType::Texture1D));
}

Texture2D::Texture2D(VkDevice device, uint32_t width, uint32_t height, uint32_t depth, TextureFormat format, TextureType type)
{
	image = VulkanAPI::CreateImage(device, width, height, depth, ToVkFormat(format), ToVkImageType(type));
	memory = VulkanAPI::AllocateImageMemory(device, image);
	view = VulkanAPI::CreateImageView(device, image, ToVkFormat(format), ToVkImageViewType(type));
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
