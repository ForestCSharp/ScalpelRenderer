#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanSwapchain
{
public:

	VulkanSwapchain();
	
	void Destroy();

	void CreateSwapchain();
	void CreateImageViews();
	vk::SwapchainKHR GetSwapchain() {return Swapchain;}

	vk::SurfaceFormatKHR ChooseSwapchainFormat();
	vk::PresentModeKHR ChooseSwapchainPresentMode();

protected:

	vk::SwapchainKHR Swapchain;

	vk::Extent2D SwapchainExtent;
	vk::Format SwapchainImageFormat;
	std::vector<vk::Image> SwapchainImages;
	std::vector<vk::ImageView> SwapchainImageViews;
};