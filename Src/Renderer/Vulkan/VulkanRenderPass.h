#pragma once

#include <vulkan/vulkan.hpp>

class VulkanRenderPass
{
public:

	VulkanRenderPass();
	
	vk::RenderPass GetRenderPass() {return RenderPass.get();}
	std::vector<vk::UniqueFramebuffer>& GetFramebuffers() { return Framebuffers; }

	void BuildRenderPass(class VulkanSwapchain& Swapchain);

protected:

	vk::UniqueRenderPass RenderPass;

	std::vector<vk::UniqueFramebuffer> Framebuffers;
};