#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanSwapchain
{
public:

	void BuildSwapchain();

	void CreateSwapchain();
	void CreateImageViews();
	void CreateDepthBuffer();
	vk::SwapchainKHR GetHandle() { return Swapchain.get(); }
	vk::Format GetSwapchainFormat() { return SwapchainImageFormat; }
	vk::Extent2D GetExtent() { return SwapchainExtent; }
	std::vector<vk::UniqueImageView>& GetImageViews() {return SwapchainImageViews;}

	vk::Format GetDepthFormat() { return DepthFormat; }
	vk::ImageView GetDepthView() { return DepthBufferView.get(); }

protected:

	vk::SurfaceFormatKHR ChooseSwapchainFormat();
	vk::PresentModeKHR ChooseSwapchainPresentMode();

protected:

	vk::UniqueSwapchainKHR Swapchain;

	vk::Extent2D SwapchainExtent;
	vk::Format SwapchainImageFormat;

	std::vector<vk::Image> SwapchainImages;
	std::vector<vk::UniqueImageView> SwapchainImageViews;

	vk::UniqueImage DepthBuffer;
	vk::UniqueDeviceMemory DepthBufferMemory;
	vk::UniqueImageView DepthBufferView;
	vk::Format DepthFormat;
};