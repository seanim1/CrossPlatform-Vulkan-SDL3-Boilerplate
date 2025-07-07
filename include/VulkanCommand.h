#pragma once
#include "Global.h"
#include <SDL3/SDL.h>
#include <array>
#include "VulkanSwapChain.h"
#include "VulkanGraphicsPipeline.h"
#include "GeometryBase.h"

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

class VulkanCommand {
private:

public:
	VulkanCommand(VkDevice logicalDevice, uint32_t queueFamilyIndex, uint32_t swapchainImageCount);
	VkCommandPool cmdPool;
	std::vector<VkCommandBuffer> frameCmdBuffers; // Command buffer storing the dispatch commands and barriers
	void buildCommandBuffers(VulkanSwapChain* swapChainX,
        VkPipelineLayout uberPipelineLayout, VkDescriptorSet uberDescSet,
        const std::vector<VulkanGraphicsPipeline*>& graphicsPipelinesX,
        const std::vector<VkBuffer>& vertexBuffers, 
        const std::vector<VkBuffer>& inIndexBuffers, 
        const std::vector<uint32_t>& indexCounts);
    PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{ VK_NULL_HANDLE };
    PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR{ VK_NULL_HANDLE };
    PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2KHR{ VK_NULL_HANDLE };

};
