#include "VulkanGraphicsPipeline.h"

#ifdef __ANDROID__
#include <SDL3/SDL_system.h>  // Required for SDL_AndroidGetAssetManager
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

AAssetManager* getAssetManagerFromSDL3() {
	JNIEnv* env = static_cast<JNIEnv*>(SDL_GetAndroidJNIEnv());
	jobject activity = static_cast<jobject>(SDL_GetAndroidActivity());

	jclass activityClass = env->GetObjectClass(activity);
	jmethodID getAssets = env->GetMethodID(activityClass, "getAssets", "()Landroid/content/res/AssetManager;");
	jobject assetManagerObj = env->CallObjectMethod(activity, getAssets);

	return AAssetManager_fromJava(env, assetManagerObj);
}
#endif

/* from vulkan-tutorial.com */
std::vector<char> VulkanGraphicsPipeline::readFile(const std::string& filename) {
#ifdef __ANDROID__
	AAssetManager* assetManager = getAssetManagerFromSDL3();
	assert(assetManager);

	AAsset* asset = AAssetManager_open(assetManager, filename.c_str(), AASSET_MODE_STREAMING);
	assert(asset);

	size_t size = AAsset_getLength(asset);
	std::vector<char> shaderCode(size);
	AAsset_read(asset, shaderCode.data(), size);
	AAsset_close(asset);

	return shaderCode;
#else
	// Desktop file loading
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
#endif
}


VkShaderModule VulkanGraphicsPipeline::createShaderModule(const std::vector<char>& code, VkDevice logicalDevice) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule{};
	if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkPhysicalDevice physicalDevice, VkDevice logicalDevice,
	VulkanSwapChain* swapChainX, VkSurfaceFormatKHR selectedSurfaceFormat,
	VkPipelineLayout uberPipelineLayout, VkPrimitiveTopology primitiveTopology, Geometry* geometry,
	VkSpecializationInfo specializationInfo,
	const std::string& vertShaderFile, const std::string& fragShaderFile)
{
	VkFormat depthStencilFormat = VulkanResourceHelper::findDepthFormat(physicalDevice);

	VkAttachmentDescription colorAttachment{}; // Color attachment
	colorAttachment.format = selectedSurfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                   // Layout to which the attachment is transitioned when the render pass is finished
	/* This means Vulkan automatically handles:
		transition from UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL at render pass start, and
		COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC_KHR at render pass end.
	*/
	VkAttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment = 0;  // Index of the attachment in the array
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth - stencil attachment
	VkAttachmentDescription depthStencilAttachment{};
	depthStencilAttachment.format = depthStencilFormat; // e.g., VK_FORMAT_D24_UNORM_S8_UINT
	depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthStencilAttachmentRef{};
	depthStencilAttachmentRef.attachment = 1;
	depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentReference;
	subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;

	std::array<VkAttachmentDescription, 2> attachments = {
		colorAttachment, depthStencilAttachment
	};

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	CATCH_ERROR(vkCreateRenderPass(logicalDevice, &renderPassInfo, 0, &renderPass));

	VulkanResourceHelper::createImage(logicalDevice, physicalDevice, swapChainX->swapChainExtent.width, swapChainX->swapChainExtent.height, depthStencilFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = VulkanResourceHelper::createImageView(logicalDevice, depthImage, depthStencilFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	swapChainFramebuffers.resize(swapChainX->swapChainImageView.size());
	for (int i = 0; i < swapChainFramebuffers.size(); i++) {
		std::array<VkImageView, 2> attachments{};
		// Color attachment is the view of the swapchain image
		attachments[0] = swapChainX->swapChainImageView[i];
		// Depth/Stencil attachment is the same for all frame buffers due to how depth works with current GPUs
		attachments[1] = depthImageView;

		VkFramebufferCreateInfo frameBufferCreateInfo{};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.renderPass = renderPass;
		frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = swapChainX->swapChainExtent.width;
		frameBufferCreateInfo.height = swapChainX->swapChainExtent.height;
		frameBufferCreateInfo.layers = 1;
		CATCH_ERROR(vkCreateFramebuffer(logicalDevice, &frameBufferCreateInfo, NULL, &swapChainFramebuffers[i]));
	}

#if defined(__IPHONE_OS_VERSION_MIN_REQUIRED)
	std::string basePath = SDL_GetBasePath(); // Returns the app bundle's Resources path
	auto vertShaderCode = readFile(basePath + "/" + vertShaderFile);
	auto fragShaderCode = readFile(basePath + "/" + fragShaderFile);
#elif defined (__ANDROID__)
	auto vertShaderCode = readFile(vertShaderFile);
	auto fragShaderCode = readFile(fragShaderFile);
#else
	std::string currentFilePath = __FILE__;

	// Find the position of the last directory separator ('/' or '\\')
	size_t pos = currentFilePath.find_last_of("/\\");

	// Extract the directory of the current source file (src/)
	std::string srcDir = currentFilePath.substr(0, pos);

	// Find the position of the last directory separator to get the root folder
	pos = srcDir.find_last_of("/\\");

	// Extract the root folder
	std::string rootDir = srcDir.substr(0, pos);
	// Load shader binary. Very much platform or development environment dependent
	auto vertShaderCode = readFile(rootDir + "/shaderBinary/" + vertShaderFile);
	auto fragShaderCode = readFile(rootDir + "/shaderBinary/" + fragShaderFile);
#endif


	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, logicalDevice);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, logicalDevice);

	VkPipelineShaderStageCreateInfo commonShaderStageInfo{};
	commonShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	commonShaderStageInfo.pSpecializationInfo = &specializationInfo;

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = commonShaderStageInfo;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = commonShaderStageInfo;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// vertexInput - inputAssembly - Vertex Shader - Tessellation - Viewport - Raster - Multisample - DepthStencil (earlyFragTest) - Fragment Shader - ColorBlend (color attachment stage)

	/* Interleaved Vertex attribute setup: [Pos, Normal, Pos, Normal, Pos, Normal] */
	VkPipelineVertexInputStateCreateInfo vertexInputState{};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(geometry->getBindingDescriptions().size());
	vertexInputState.pVertexBindingDescriptions = geometry->getBindingDescriptions().data();
	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(geometry->getAttributeDescriptions().size());
	vertexInputState.pVertexAttributeDescriptions = geometry->getAttributeDescriptions().data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = primitiveTopology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// We either need this or define it in the pDynamicState, but one of them is required
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainX->swapChainExtent.width;
	viewport.height = (float)swapChainX->swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainX->swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// (We don't need it but, required, otherwise validation error)
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// (We don't need it but, required, otherwise validation error)
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// Dynamic rendering attachment formats (there is no longer a need for VkRenderPass and VkFramebuffer)
	VkPipelineRenderingCreateInfo dynamicRendering{};
	dynamicRendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	dynamicRendering.colorAttachmentCount = 1;
	dynamicRendering.pColorAttachmentFormats = &selectedSurfaceFormat.format;
	dynamicRendering.depthAttachmentFormat = VulkanResourceHelper::findDepthFormat(physicalDevice);

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = &dynamicRendering;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputState;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = uberPipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}
