#pragma once

#include <vector>
#include <map>
#include <vulkan/vulkan.hpp>
#include "VulkanCommandBuffer.h"
#include "../Framework/RenderItem.hpp"
#include "VulkanGraphicsPipeline.h"

class VulkanRenderPass
{
public:

	VulkanRenderPass();
	
	vk::RenderPass GetHandle() {return RenderPass.get();}
	std::vector<vk::UniqueFramebuffer>& GetFramebuffers() { return Framebuffers; }

	void BuildRenderPass(class VulkanSwapchain& Swapchain);

	//Builds a secondary command buffer for this render pass
	void BuildCommandBuffer(std::vector<std::pair<RenderItem*, VulkanGraphicsPipeline*>> ItemsToRender, vk::DescriptorSet TEST_DESC_SET);
	VulkanCommandBuffer& GetCommandBuffer() { return CommandBuffer; }

protected:

	vk::UniqueRenderPass RenderPass;

	std::vector<vk::UniqueFramebuffer> Framebuffers;

	/** Secondary command buffer that orchestrates bipeline binds and render calls */
	VulkanCommandBuffer CommandBuffer;
};