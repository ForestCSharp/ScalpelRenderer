#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>

class VulkanSwapchain
{
public:

	VulkanSwapchain(struct GLFWwindow* window);
	
	void Destroy();

	//Platform-Specific Surface Creation
    void CreateGLFWSurface(struct GLFWwindow* window);
	vk::SurfaceKHR GetSurface() {return Surface;}

	void CreateSwapchain();
	vk::SwapchainKHR GetSwapchain() {return Swapchain;}

	vk::SurfaceFormatKHR ChooseSwapchainFormat();
	vk::PresentModeKHR ChooseSwapchainPresentMode();

protected:

	vk::SurfaceKHR Surface;
	vk::SwapchainKHR Swapchain;

	vk::Extent2D SwapchainExtent;
	std::vector<vk::Image> SwapchainImages;
	vk::Format SwapchainImageFormat;

};