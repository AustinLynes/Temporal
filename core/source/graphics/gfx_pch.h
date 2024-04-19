#pragma once

#include <pch.h>
#include <datastructures/datastructures_pch.h>

#include <graphics/vulkan_api.h>

struct Resolution {
	uint32_t width;
	uint32_t height;
};


#ifdef max 
#undef max
#endif
#ifdef min
#undef min
#endif
