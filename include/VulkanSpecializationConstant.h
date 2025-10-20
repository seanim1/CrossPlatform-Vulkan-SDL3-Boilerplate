#pragma once
#include "Global.h"
#include "GeometryBase.h"
#include <vector>
#include <fstream>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#ifdef USE_GPU
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>
#if defined(_WIN32)
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#elif defined(__linux__)
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
#elif defined(__linux__) && defined(USING_WAYLAND)
#include <wayland-client.h>
#include <vulkan/vulkan_wayland.h>
#elif defined(__APPLE__) // For both MacOS and iOS
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_beta.h>
#endif
#endif

typedef struct GpuConstantData {
	int Screen_Width;
	int Screen_Heigt;
	int model_id;
} GpuConstantData;

class VulkanSpecializationConstant {
private:
public:
	VulkanSpecializationConstant(
		int Screen_Width,
		int Screen_Heigt,
		int model_id
	);
	static constexpr int specialization_constant_count = (sizeof(GpuConstantData) / 4);  // static, same across all instances
	VkSpecializationMapEntry specializationMapEntries[specialization_constant_count]{};
	VkSpecializationInfo specializationInfo{};
	GpuConstantData gpuConstantData{};
};
