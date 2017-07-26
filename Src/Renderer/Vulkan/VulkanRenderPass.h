
#include <vulkan/vulkan.hpp>

class VulkanRenderPass
{
public:

	VulkanRenderPass();
	
	vk::RenderPass GetRenderPass() {return RenderPass.get();}

	void BuildRenderPass(class VulkanSwapchain& Swapchain);

protected:

	vk::UniqueRenderPass RenderPass;
};