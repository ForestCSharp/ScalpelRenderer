#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanSwapchain
{
public:

	VulkanSwapchain();
	
	void Destroy();

	void CreateSwapchain();
	vk::SwapchainKHR GetSwapchain() {return Swapchain;}

	vk::SurfaceFormatKHR ChooseSwapchainFormat();
	vk::PresentModeKHR ChooseSwapchainPresentMode();

protected:

	vk::SwapchainKHR Swapchain;

	vk::Extent2D SwapchainExtent;
	std::vector<vk::Image> SwapchainImages;
	vk::Format SwapchainImageFormat;

protected:

	uint32_t PresentQueueIndex;
	vk::Queue PresentQueue;
};