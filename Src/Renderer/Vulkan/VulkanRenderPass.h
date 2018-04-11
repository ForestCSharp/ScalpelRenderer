#pragma once

#include <vulkan/vulkan.hpp>

class VulkanRenderPass
{
public:

	VulkanRenderPass();
	
	vk::RenderPass GetHandle() {return RenderPass.get();}
	std::vector<vk::UniqueFramebuffer>& GetFramebuffers() { return Framebuffers; }

	void BuildRenderPass(class VulkanSwapchain& Swapchain);

	//Builds a secondary command buffer for this render pass
	void BuildCommands();

protected:

	vk::UniqueRenderPass RenderPass;

	std::vector<vk::UniqueFramebuffer> Framebuffers;

	/** Secondary command buffer that orchestrates bipeline binds and render calls */
	class VulkanCommandBuffer CommandBuffer;
};