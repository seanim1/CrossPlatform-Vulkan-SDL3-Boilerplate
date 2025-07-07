#include "VulkanCommand.h"

VulkanCommand::VulkanCommand(VkDevice logicalDevice, uint32_t queueFamilyIndex, uint32_t swapchainImageCount)
{
    VkCommandPoolCreateInfo cmdPoolInfo{};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.queueFamilyIndex = 0;
    cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CATCH_ERROR(vkCreateCommandPool(logicalDevice, &cmdPoolInfo, NULL, &cmdPool));

    frameCmdBuffers.resize(swapchainImageCount);
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = cmdPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t) frameCmdBuffers.size();
    // Command buffer for each frame
    CATCH_ERROR(vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, frameCmdBuffers.data()));

    // Since we use an extension, we need to expliclity load the function pointers for extension related Vulkan commands
    vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetDeviceProcAddr(logicalDevice, "vkCmdBeginRenderingKHR"));
    vkCmdEndRenderingKHR = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetDeviceProcAddr(logicalDevice, "vkCmdEndRenderingKHR"));
    vkCmdPipelineBarrier2KHR = reinterpret_cast<PFN_vkCmdPipelineBarrier2KHR>(vkGetDeviceProcAddr(logicalDevice, "vkCmdPipelineBarrier2KHR"));

}
static void init_vkDependencyInfo(VkDependencyInfo* dependencyInfoAddress) {
    dependencyInfoAddress->sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfoAddress->dependencyFlags = 0;
    dependencyInfoAddress->memoryBarrierCount = 0;
    dependencyInfoAddress->pMemoryBarriers = NULL;
    dependencyInfoAddress->bufferMemoryBarrierCount = 0;
    dependencyInfoAddress->pBufferMemoryBarriers = NULL;
    dependencyInfoAddress->imageMemoryBarrierCount = 0;
    dependencyInfoAddress->pImageMemoryBarriers = NULL;
}

void VulkanCommand::buildCommandBuffers(VulkanSwapChain* swapChainX,
    VkPipelineLayout uberPipelineLayout, VkDescriptorSet uberDescSet,
    const std::vector<VulkanGraphicsPipeline*>& graphicsPipelinesX,
    const std::vector<VkBuffer>& inVertexBuffers,
    const std::vector<VkBuffer>& inIndexBuffers,
    const std::vector<uint32_t>& indexBufferCounts)
{
    VkImageSubresourceRange subresourceRange_default;
    subresourceRange_default.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange_default.baseMipLevel = 0;
    subresourceRange_default.levelCount = 1;
    subresourceRange_default.baseArrayLayer = 0;
    subresourceRange_default.layerCount = 1;

    VkImageMemoryBarrier imgBar_None2ColAtt_Undef2ColAtt{};
    imgBar_None2ColAtt_Undef2ColAtt.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgBar_None2ColAtt_Undef2ColAtt.srcAccessMask = VK_ACCESS_NONE;
    imgBar_None2ColAtt_Undef2ColAtt.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imgBar_None2ColAtt_Undef2ColAtt.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgBar_None2ColAtt_Undef2ColAtt.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imgBar_None2ColAtt_Undef2ColAtt.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBar_None2ColAtt_Undef2ColAtt.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBar_None2ColAtt_Undef2ColAtt.subresourceRange = subresourceRange_default;

    VkImageMemoryBarrier imgBar_ColAtt2None_ColAtt2PresentSrc{};
    imgBar_ColAtt2None_ColAtt2PresentSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imgBar_ColAtt2None_ColAtt2PresentSrc.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imgBar_ColAtt2None_ColAtt2PresentSrc.dstAccessMask = VK_ACCESS_NONE;
    imgBar_ColAtt2None_ColAtt2PresentSrc.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imgBar_ColAtt2None_ColAtt2PresentSrc.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imgBar_ColAtt2None_ColAtt2PresentSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBar_ColAtt2None_ColAtt2PresentSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBar_ColAtt2None_ColAtt2PresentSrc.subresourceRange = subresourceRange_default;

    VkCommandBuffer frameCommandBuffer{};
    for (int swapChainImageIndex = 0; swapChainImageIndex < frameCmdBuffers.size(); swapChainImageIndex++) {
        frameCommandBuffer = frameCmdBuffers[swapChainImageIndex];
        // Command Buffer State: Initial
        vkResetCommandBuffer(frameCommandBuffer, 0);
        VkCommandBufferBeginInfo cmdBufferBeginInfo{};
        cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // Command Buffer State: Recording (vkCmd* commands can be used to record to the command buffer)
        vkBeginCommandBuffer(frameCommandBuffer, &cmdBufferBeginInfo);

        vkCmdBindDescriptorSets(frameCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, uberPipelineLayout, 0, 1, &uberDescSet, 0, nullptr);

        imgBar_None2ColAtt_Undef2ColAtt.image = swapChainX->swapChainImages[swapChainImageIndex];
        //vkCmdPipelineBarrier(
        //    frameCommandBuffer,
        //    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        //    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        //    0,
        //    0, nullptr,
        //    0, nullptr,
        //    1, &imgBar_None2ColAtt_Undef2ColAtt
        //);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = graphicsPipelinesX[0]->renderPass;
        renderPassInfo.framebuffer = graphicsPipelinesX[0]->swapChainFramebuffers[swapChainImageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainX->swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.0f, 0.0f, 0.15f, 1.0f };
        //clearValues[0].color = { 240. / 255. , 188. / 255., 158. / 255., 1.0f };

        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(frameCommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        for (int i = 0; i < indexBufferCounts.size(); i++) {
            vkCmdBindPipeline(frameCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphicsPipelinesX[i]->graphicsPipeline);
            VkBuffer vertexBuffer[] = { inVertexBuffers.data()[i] };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(frameCmdBuffers[swapChainImageIndex], 0,
                1, vertexBuffer, offsets);
            vkCmdBindIndexBuffer(frameCmdBuffers[swapChainImageIndex], inIndexBuffers[i], 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(frameCmdBuffers[swapChainImageIndex], indexBufferCounts[i], 1, 0, 0, 0);
        }

        vkCmdDraw(frameCommandBuffer, 3, 1, 0, 0);

        vkCmdEndRenderPass(frameCommandBuffer);

        imgBar_ColAtt2None_ColAtt2PresentSrc.image = swapChainX->swapChainImages[swapChainImageIndex];
        //vkCmdPipelineBarrier(
        //    frameCommandBuffer,
        //    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
        //    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,           // dstStageMask
        //    0,
        //    0, nullptr,
        //    0, nullptr,
        //    1, &imgBar_ColAtt2None_ColAtt2PresentSrc
        //);

        if (vkEndCommandBuffer(frameCmdBuffers[swapChainImageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

}
