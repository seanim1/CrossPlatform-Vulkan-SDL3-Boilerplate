#pragma once
#include "Global.h"
#include "GeometryBase.h"
#include <vector>
#include <array>
#include <fstream>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include "VulkanResourceHelpers.h"
#include "VulkanSwapChain.h"

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
class VulkanGraphicsPipeline {
private:
	VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice logicalDevice);
	std::vector<char> readFile(const std::string& filename);
public:
	VulkanGraphicsPipeline(VkPhysicalDevice physicalDevice, VkDevice logicalDevice,
		VulkanSwapChain* swapChainX, VkSurfaceFormatKHR selectedSurfaceFormat,
		VkPipelineLayout uberPipelineLayout, VkPrimitiveTopology primitiveTopology, Geometry* geometry,
		VkSpecializationInfo specializationInfo,
		const std::string& vertShaderFile, const std::string& fragShaderFile);
	VkPipeline graphicsPipeline;
	VkImageView depthImageView;
	VkDeviceMemory depthImageMemory;
	VkImage depthImage;
	// non-dynamic rendering
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> swapChainFramebuffers; // where graphics pipeline output image will be rendered to

};
