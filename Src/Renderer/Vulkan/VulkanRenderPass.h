#pragma once

#include <vector>
#include <map>
#include <vulkan/vulkan.hpp>
#include "VulkanCommandBuffer.h"
#include "../Framework/RenderItem.hpp"
#include "VulkanGraphicsPipeline.h"
#include "VulkanImage.h"

struct VulkanRenderTarget
{
	//TODO: 1 Image per back buffer
	std::vector<vk::ImageView*> ImageViews;
	vk::Format Format;

	vk::AttachmentLoadOp LoadOp;
	vk::AttachmentStoreOp StoreOp;

	vk::ImageLayout InitialLayout;
	vk::ImageLayout UsageLayout;
	vk::ImageLayout FinalLayout;

	/*TODO: Better way to handle this, perhaps a second argument in build renderpass, 
			that always gets appended to the end of our attachment array */
	bool bDepthTarget = false;
};

class VulkanRenderPass
{
public:

	VulkanRenderPass();
	
	vk::RenderPass GetHandle() {return RenderPass.get();}

	std::vector<vk::UniqueFramebuffer>& GetFramebuffers() { return Framebuffers; }

	//Used by frame graph to build this render target
	void BuildRenderPass(std::vector<VulkanRenderTarget*> RenderTargets, uint32_t Width, uint32_t Height, uint32_t BackbufferCount);

	//Builds a secondary command buffer for this render pass
	void BuildCommandBuffer(std::vector<std::pair<RenderItem*, VulkanGraphicsPipeline*>> ItemsToRender, vk::DescriptorSet TEST_DESC_SET);
	VulkanCommandBuffer& GetCommandBuffer() { return CommandBuffer; }

	//TODO: Function that Adds commands for this RenderPass to input Command Buffer

protected:

	vk::UniqueRenderPass RenderPass;

	std::vector<vk::UniqueFramebuffer> Framebuffers;

	/** Secondary command buffer that orchestrates bipeline binds and render calls */
	VulkanCommandBuffer CommandBuffer;
};