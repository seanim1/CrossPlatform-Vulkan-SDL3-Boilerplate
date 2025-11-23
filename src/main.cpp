   #define SDL_MAIN_USE_CALLBACKS 1

#include "Global.h"
#include "GameWindow.h"
#include "GameScreen.h"
#include "GameCamera.h"
#include "GameInput.h"
#include "GameTimer.h"

#include "GeometryLine.h"
#include "GeometryBox.h"
#include "GeometryQuad.h"
#include "GeometryGrid.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp> // Required for glm::to_string
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_main_impl.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_video.h>

#ifdef USE_GPU
#define VALIDATION_LAYER_VULKAN
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"
#include "VulkanSwapChain.h"
#include "VulkanCommand.h"
#include "VulkanSynchronization.h"
#include "VulkanSpecializationConstant.h"
#include "VulkanDescBufferUniform.h"
#include "VulkanUberDescriptorSet.h"
#include "VulkanGraphicsPipeline.h"

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

#include <string>

static GameWindow* gWindow = nullptr;
static GameScreen* gScreen = nullptr;
static GameCamera* gCamera = nullptr;
static GameInput* gInput = nullptr;
static GameTimer* gTimer = nullptr;
static VulkanQueue* queueX = nullptr;
static VulkanDevice* deviceX = nullptr;
static VulkanSynchronization* syncX = nullptr;
static VulkanCommand* cmdX = nullptr;
static VulkanSwapChain* swapChainX = nullptr;
static std::vector<VulkanDesc*> descriptorList;

// TICKET: figure out why the third Geo is being rendered so strangily
constexpr size_t GEO_CNT = 3; 
// this struct maps to Global.comp's layout(std430, binding = 0) uniform UniformBufferObject.
// I also hardcoded the 
struct UboCameraMatrices {
	glm::mat4 models[GEO_CNT]; // this array size must be updated accordingly in Global.comp
	glm::mat4 view;
	glm::mat4 proj;

	glm::vec3 camPos;
	float elapsedTime;
	
	glm::vec3 camDir;
	float padding_0;

	glm::vec3 dirLightDir;
	float padding_1;
};
static UboCameraMatrices camUbo;
static std::vector<Geometry*> geos;
static std::vector<Line*> lines;
static std::vector<Box*> boxes;
static std::vector<Quad*> quads;
static std::vector<Grid*> grids;
/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	SDL_SetLogPriority(SDL_LOG_CATEGORY_INPUT, SDL_LOG_PRIORITY_CRITICAL);
    gWindow = new GameWindow(1280, 720, "Cross-Platform GUI");
    gScreen = new GameScreen(gWindow->dimension.x, gWindow->dimension.y, gWindow->renderer);
	gCamera = new GameCameraPerspective(60.0f, gWindow->dimension.x / static_cast<float>(gWindow->dimension.y));
	//gCamera = new GameCameraOrthographic(gWindow->dimension.x / static_cast<float>(gWindow->dimension.y));
	gInput = new GameInput();
    gTimer = new GameTimer();
	
#ifdef USE_GPU
	std::vector<const char*> requestingInstanceExtensions = {
	VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
	VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
	};
	// VK_KHR_SURFACE_EXTENSION_NAME and platform specific surface will be provided by SDL
	// Get required Vulkan extensions
	uint32_t SDL_VulkanExtensionCount = 0;
	const char* const* SDL_VulkanExtensions = SDL_Vulkan_GetInstanceExtensions(&SDL_VulkanExtensionCount);
	SDL_Log("SDL3 Vulkan instance extensions required:");
	for (uint32_t i = 0; i < SDL_VulkanExtensionCount; i++) {
		SDL_Log("- %s", SDL_VulkanExtensions[i]);
		requestingInstanceExtensions.push_back(SDL_VulkanExtensions[i]);
	}
#if defined(__APPLE__)
	requestingInstanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME); // vkCreateInstance will fail with VK_ERROR_INCOMPATIBLE_DRIVER: https://stackoverflow.com/questions/72789012/why-does-vkcreateinstance-return-vk-error-incompatible-driver-on-macos-despite
#endif
	std::vector<const char*> requestingInstanceLayers = {
#ifdef VALIDATION_LAYER_VULKAN
		"VK_LAYER_KHRONOS_validation"
#endif
	};
	std::vector<const char*> requestedDeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, // for VK_ACCESS_NONE in the pipeline barrier
	VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME,  // for GL_EXT_shader_explicit_arithmetic_types_float16
	VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
	VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
	VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME,
	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
	VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, // for shader printf: GL_EXT_debug_printf
	VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME,
	VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME, // not fully supported on RX6600 (imageAtomicAdd not supported)
	//VK_EXT_SHADER_IMAGE_ATOMIC_INT64_EXTENSION_NAME,
	};
#if defined(__APPLE__)
	// From SaschaWillems - When running on iOS/macOS with MoltenVK and VK_KHR_portability_subset is defined and supported by the device, enable the extension
	requestedDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif
	VulkanInstance* instanceX = new VulkanInstance(&requestingInstanceExtensions, &requestingInstanceLayers);
	VulkanPhysicalDevice* physicalDeviceX = new VulkanPhysicalDevice(instanceX->instance);

	queueX = new VulkanQueue(0, 1);
	deviceX = new VulkanDevice(physicalDeviceX->physicalDevice, physicalDeviceX->computeQueueFamilyIndex, &requestedDeviceExtensions);
	queueX->initQueue(deviceX->logicalDevice);
	instanceX->initSurface(gWindow->window);
	swapChainX = new VulkanSwapChain(physicalDeviceX->physicalDevice, instanceX->surface, deviceX->logicalDevice);
	cmdX = new VulkanCommand(deviceX->logicalDevice, queueX->queueFamilyIndex, (uint32_t) swapChainX->swapChainImages.size());
	syncX = new VulkanSynchronization(deviceX->logicalDevice);
	//geos.push_back( new Quad(2.f, 2.f) );
	//geos.push_back( new Box(1.7f, 1.7f, 1.7f) );
	//geos.push_back( new Box(1.7f, 1.7f, 1.7f) );
	float quad_size = 1.f;
	geos.push_back(new Quad( quad_size, quad_size ) );
	geos.push_back(new Quad( quad_size, quad_size ) );
	geos.push_back(new Quad( quad_size, quad_size ) );

	/* <Instantiate descriptorList>
	* descriptorList: [ uniformBuffer | Box 0 Vertex Buffer | Box 0 Index Buffer | Box 1 Vertex Buffer | Box 1 Index Buffer | ... ]
	*/
	descriptorList.push_back( new VulkanDescBufferUniform(&camUbo, sizeof(camUbo), deviceX->logicalDevice, physicalDeviceX->physicalDevice));
	for (int i = 0; i < geos.size(); i++) {
		descriptorList.push_back(new VulkanDescBuffer(deviceX->logicalDevice, physicalDeviceX->physicalDevice,
			geos[i]->getVertexData(), geos[i]->getVertexCount() * geos[i]->getVertexStride(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT));
		descriptorList.push_back(new VulkanDescBuffer(deviceX->logicalDevice, physicalDeviceX->physicalDevice,
			geos[i]->getIndexData(), geos[i]->getIndexCount() * sizeof(geos[i]->getIndexData()[0]),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT));
	}

	/* <Allocate descriptors: descriptorList>
	*/
	((VulkanDescBufferUniform*)descriptorList[0])->allocateUniformBuffer(deviceX->logicalDevice, physicalDeviceX->physicalDevice);
	for (int i = 1; i < descriptorList.size(); i++) {
		((VulkanDescBuffer*)descriptorList[i])->allocate(deviceX->logicalDevice, physicalDeviceX->physicalDevice, cmdX->cmdPool, queueX->queue);
	}

	VulkanUberDescriptorSet* descriptorX = new VulkanUberDescriptorSet(deviceX->logicalDevice, descriptorList);
	
	std::vector<VulkanGraphicsPipeline*> graphicsPipelines;

	/* <Initialize graphics pipelines> such that each Geometry runs a different shader
	*/
	std::vector<std::string> shaderBaseNames = {
	"2d_background_circles_repeat",
	"2d_background_circles_repeat",
	"2d_background_circles_repeat",
	};
	std::string suffixFrag = ".frag.spv";
	std::string suffixVert = ".vert.spv";

	// Loop from 0 to 2 (or any number of files you need)
	for (int i = 0; i < geos.size(); i++) {

		VulkanSpecializationConstant* specialConstantX = new VulkanSpecializationConstant(
			gScreen->dimension.x,
			gScreen->dimension.y,
			i // Model Index ID
		);

		// 2. Concatenate the parts to form the full fragfilename.
		std::string vertfilename = "default_camera" + suffixVert;
		std::string fragfilename = shaderBaseNames[i] + suffixFrag;

		// 3. Output the result (or use it to load the file)
		std::cout << "Generated fragfilename: " << fragfilename << std::endl;

		graphicsPipelines.push_back(new VulkanGraphicsPipeline(physicalDeviceX->physicalDevice, deviceX->logicalDevice,
			swapChainX, swapChainX->selectedSurfaceFormat,
			descriptorX->uberPipelineLayout, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, geos[i],
			specialConstantX->specializationInfo,
			vertfilename, fragfilename));
	}

	/*	<Initialize vertex buffer>
	*	<Initialize index buffer>
	*	The following three buffers are guaranteed to be of same size, same size as the geos[].
	*/
	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkBuffer> indexBuffers;
	std::vector<uint32_t> indexBufferCounts;
	int counter_for_nextIndexBuffer = 0;
	for (int i = 0; i < descriptorList.size(); i++) {
		VulkanDescBuffer* descriptor = (VulkanDescBuffer*)descriptorList[i];
		VkBufferUsageFlags bufferUsageFlags = descriptor->usageFlags;
		if ((bufferUsageFlags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) == VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
			vertexBuffers.push_back(descriptor->buffer);
		}
		else if ((bufferUsageFlags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) == VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
			indexBuffers.push_back(descriptor->buffer);
			indexBufferCounts.push_back(geos[counter_for_nextIndexBuffer]->getIndexCount());
			counter_for_nextIndexBuffer++;
		}
	}

	cmdX->buildCommandBuffers(swapChainX, descriptorX->uberPipelineLayout,
		descriptorX->uberDescSet, graphicsPipelines,
		vertexBuffers, indexBuffers, indexBufferCounts);

#endif

	gCamera->SetPosition(glm::vec3(0., 0., -8.0));
	geos[0]->setRotation(glm::vec3(glm::pi<float>(), 0., 0.));
	geos[1]->setRotation(glm::vec3(glm::pi<float>(), 0., 0.));
	geos[2]->setRotation(glm::vec3(glm::pi<float>(), 0., 0.));
	geos[0]->setPosition(glm::vec3(quad_size * 1.1f, 0., 0.));
	geos[1]->setPosition(glm::vec3(0., 0., 0.));
	geos[2]->setPosition(glm::vec3(-quad_size * 1.1f, 0., 0.));

    return SDL_APP_CONTINUE; // SDL_APP_FAILURE to indicate failure
}

/* This function runs when SDL receives input or OS-level events. */
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; // Clean exit
    }
    gInput->ProcessEvent(*event);
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame. */
SDL_AppResult SDL_AppIterate(void* appstate) {
    gTimer->StartTimer();

	glm::vec3 delta_pos(0.0f);
	const std::array<bool, SDL_SCANCODE_COUNT> keys = gInput->getKeys();
	if (keys[SDL_SCANCODE_W]) {
		SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT, "Holding W");
		delta_pos.z += 0.1f;
	}
	if (keys[SDL_SCANCODE_A]) {
		SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT, "Holding A");
		delta_pos.x += 0.1f;
	}
	if (keys[SDL_SCANCODE_S]) {
		SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT, "Holding S");
		delta_pos.z -= 0.1f;
	}
	if (keys[SDL_SCANCODE_D]) {
		SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT, "Holding D");
		delta_pos.x -= 0.1f;
	}
	if (keys[SDL_SCANCODE_Q]) {
		SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT, "Holding Q");
		delta_pos.y -= 0.1f;
	}
	if (keys[SDL_SCANCODE_E]) {
		SDL_LogVerbose(SDL_LOG_CATEGORY_INPUT, "Holding E");
		delta_pos.y += 0.1f;
	}
	gCamera->SetPosition(gCamera->GetPosition() + delta_pos);
	SDL_Log("cameraPos: %s", glm::to_string(gCamera->GetPosition()).c_str());
	// switch between perspective and orthographic
	//if (keys[SDL_SCANCODE_P]) {
	//	gCamera = new GameCameraPerspective(60.0f, gWindow->dimension.x / static_cast<float>(gWindow->dimension.y));
	//}
	//if (keys[SDL_SCANCODE_O]) {
	//	gCamera = new GameCameraOrthographic(gWindow->dimension.x / static_cast<float>(gWindow->dimension.y));
	//}

#ifdef USE_GPU
	for (int i = 0; i < GEO_CNT; i++) {
		camUbo.models[i] = geos[i]->getModelMatrix();
	}
	if (keys[SDL_SCANCODE_T]) {
		gCamera->SetDirConfig(GameCamera::TARGET_CAM);
	}
	if (keys[SDL_SCANCODE_R]) {
		gCamera->SetDirConfig(GameCamera::FREE_CAM);
	}
	glm::vec3 newCamRot = gCamera->GetRotation();

	float cameraSensitivity = 0.4;
	if (keys[SDL_SCANCODE_J]) {
		newCamRot.x += cameraSensitivity;
	}
	if (keys[SDL_SCANCODE_L]) {
		newCamRot.x -= cameraSensitivity;
	}
	if (keys[SDL_SCANCODE_K]) {
		newCamRot.y += cameraSensitivity;
	}
	if (keys[SDL_SCANCODE_I]) {
		newCamRot.y -= cameraSensitivity;
	}
	if (keys[SDL_SCANCODE_O]) {
		newCamRot.z += cameraSensitivity;
	}
	if (keys[SDL_SCANCODE_U]) {
		newCamRot.z -= cameraSensitivity;
	}
	gCamera->SetRotation(newCamRot);
	camUbo.view = gCamera->GetViewMatrix();
	camUbo.proj = gCamera->GetProjectionMatrix();
	camUbo.camPos = gCamera->GetPosition();
	camUbo.elapsedTime = gTimer->elapsedTime;
	camUbo.camDir = gCamera->GetDirection();
	//SDL_Log("cameraDir: %s", glm::to_string(gCamera->GetDirection()).c_str());

	camUbo.dirLightDir = glm::normalize(glm::vec3(2.5, 25.0 * cos(gTimer->elapsedTime), 25.0 * sin(gTimer->elapsedTime)));
	((VulkanDescBufferUniform*)descriptorList[0])->update();
    // Vulkan rendering goes here
	queueX->drawFrame(deviceX->logicalDevice, syncX, swapChainX->swapChain, cmdX);
#else
    gScreen->Update(gTimer->elapsedTime, gWindow->renderer);
#endif

    gTimer->EndTimer();
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
    delete gTimer;
    delete gInput;
    delete gScreen;
    delete gWindow;
}
