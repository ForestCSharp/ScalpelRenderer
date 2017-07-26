#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanSwapchain
{
public:

	VulkanSwapchain();

	void CreateSwapchain();
	void CreateImageViews();
	vk::SwapchainKHR GetSwapchain() { return Swapchain.get(); }
	vk::Format GetFormat() { return SwapchainImageFormat; }
	vk::Extent2D GetExtent() { return SwapchainExtent; }

	vk::SurfaceFormatKHR ChooseSwapchainFormat();
	vk::PresentModeKHR ChooseSwapchainPresentMode();

protected:

	vk::UniqueSwapchainKHR Swapchain;

	vk::Extent2D SwapchainExtent;
	vk::Format SwapchainImageFormat;
	std::vector<vk::Image> SwapchainImages;
	std::vector<vk::UniqueImageView> SwapchainImageViews;
};