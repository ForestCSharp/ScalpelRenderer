#include "VulkanSwapchain.h"

#include "VulkanContext.h"
#include <iostream>
#include <vector>
#include <glfw/glfw3.h>

VulkanSwapchain::VulkanSwapchain(GLFWwindow* window)
{
	CreateGLFWSurface(window);
	CreateSwapchain();
}

void VulkanSwapchain::Destroy()
{
	std::cout << "--- DESTROY SURFACE AND SWAPCHAIN ---" << std::endl;
	VulkanContext::Get()->GetDevice().destroySwapchainKHR(Swapchain);
	VulkanContext::Get()->GetInstance().destroySurfaceKHR(Surface);
}

void VulkanSwapchain::CreateGLFWSurface(GLFWwindow* window)
{
    VkSurfaceKHR tmp;
    if (glfwCreateWindowSurface(VulkanContext::Get()->GetInstance(), window, nullptr, &tmp) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create window surface!");
    }

    std::cout << "GLFW Surface Successfully Created" << std::endl;
    Surface = tmp;

	//Ensure our graphics queue also has presentation support 
	VkBool32 presentSupport = false;
	VulkanContext::Get()->GetPhysicalDevice().getSurfaceSupportKHR(VulkanContext::Get()->GetGraphicsQueueIndex(), Surface, &presentSupport);
	if (!presentSupport)
	{
		std::cout << "ERROR: Graphics Queue doesn't support presentation" << std::endl;
	}
	// (TODO: use secondary present queue as fallback)
}

void VulkanSwapchain::CreateSwapchain()
{
	vk::SurfaceFormatKHR DesiredFormat = ChooseSwapchainFormat();
	vk::PresentModeKHR PresentMode = ChooseSwapchainPresentMode();

	vk::SurfaceCapabilitiesKHR SurfaceCapabilities = VulkanContext::Get()->GetPhysicalDevice().getSurfaceCapabilitiesKHR(Surface);

	uint32_t ImageCount = SurfaceCapabilities.minImageCount + 1;
	if (SurfaceCapabilities.maxImageCount > 0 && ImageCount > SurfaceCapabilities.maxImageCount) 
	{
		ImageCount = SurfaceCapabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR CreateInfo;
	CreateInfo.surface = Surface;
	CreateInfo.minImageCount = ImageCount;
	CreateInfo.imageFormat = DesiredFormat.format;
	SwapchainImageFormat = CreateInfo.imageFormat; //Store for later
	CreateInfo.imageColorSpace = DesiredFormat.colorSpace;
	CreateInfo.imageExtent = SurfaceCapabilities.currentExtent;
	SwapchainExtent = CreateInfo.imageExtent; //Store for later

	CreateInfo.imageArrayLayers = 1;
	CreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

	//TODO: Allow Separate Graphics and Presentation Queues
	uint32_t GraphicsQueueIndex = VulkanContext::Get()->GetGraphicsQueueIndex();
	CreateInfo.imageSharingMode = vk::SharingMode::eExclusive;

	CreateInfo.preTransform = SurfaceCapabilities.currentTransform;
	CreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	CreateInfo.presentMode = PresentMode;

	Swapchain = VulkanContext::Get()->GetDevice().createSwapchainKHR(CreateInfo);
	
	SwapchainImages = VulkanContext::Get()->GetDevice().getSwapchainImagesKHR(Swapchain);

	std::cout << "Swapchain Successfully created" << std::endl;
}

vk::SurfaceFormatKHR VulkanSwapchain::ChooseSwapchainFormat()
{
	std::vector<vk::SurfaceFormatKHR> SurfaceFormats = VulkanContext::Get()->GetPhysicalDevice().getSurfaceFormatsKHR(Surface);
	vk::SurfaceFormatKHR DesiredFormat;

	//Try to find our desired format
	for (const auto& availableFormat : SurfaceFormats) 
	{
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm
			&& availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) 
		{
			DesiredFormat = availableFormat;
		}
	}
	
	if (SurfaceFormats.size() == 1)
	{
		//Free to choose our own format and color space
		if (SurfaceFormats[0].format == vk::Format::eUndefined)
		{
			DesiredFormat.format = vk::Format::eB8G8R8A8Unorm;
			DesiredFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
			return DesiredFormat;
		}
		else //Just use first version
		{
			DesiredFormat = SurfaceFormats[0];
		}
	}

	return DesiredFormat;
}

vk::PresentModeKHR VulkanSwapchain::ChooseSwapchainPresentMode()
{
	std::vector<vk::PresentModeKHR> SurfacePresentModes = VulkanContext::Get()->GetPhysicalDevice().getSurfacePresentModesKHR(Surface);

	vk::PresentModeKHR bestMode;

	for (const auto& availablePresentMode : SurfacePresentModes) 
	{
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) 
		{
            return availablePresentMode;
        }
		else if (availablePresentMode == vk::PresentModeKHR::eImmediate) 
		{
            bestMode = availablePresentMode;
        }
    }

	return bestMode;
}