#include "VulkanSwapchain.h"

#include "VulkanContext.h"
#include <iostream>
#include <vector>
#include <glfw/glfw3.h>

void VulkanSwapchain::BuildSwapchain()
{
	CreateSwapchain();
	CreateImageViews();
	CreateDepthBuffer();
}

void VulkanSwapchain::CreateSwapchain()
{
	vk::SurfaceFormatKHR DesiredFormat = ChooseSwapchainFormat();
	vk::PresentModeKHR PresentMode = ChooseSwapchainPresentMode();

	vk::SurfaceCapabilitiesKHR SurfaceCapabilities = VulkanContext::Get()->GetPhysicalDevice().getSurfaceCapabilitiesKHR(VulkanContext::Get()->GetSurface());

	uint32_t ImageCount = SurfaceCapabilities.minImageCount + 1;
	if (SurfaceCapabilities.maxImageCount > 0 && ImageCount > SurfaceCapabilities.maxImageCount) 
	{
		ImageCount = SurfaceCapabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR CreateInfo;
	CreateInfo.surface = VulkanContext::Get()->GetSurface();
	CreateInfo.minImageCount = ImageCount;
	CreateInfo.imageFormat = DesiredFormat.format;
	SwapchainImageFormat = CreateInfo.imageFormat; //Store for later
	CreateInfo.imageColorSpace = DesiredFormat.colorSpace;
	CreateInfo.imageExtent = SurfaceCapabilities.currentExtent;
	SwapchainExtent = CreateInfo.imageExtent; //Store for later

	CreateInfo.imageArrayLayers = 1;
	CreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	
	uint32_t queueFamilyIndices[] = {(uint32_t)VulkanContext::Get()->GetGraphicsQueueIndex(), (uint32_t)VulkanContext::Get()->GetPresentQueueIndex()};
	if (VulkanContext::Get()->GetGraphicsQueueIndex() != VulkanContext::Get()->GetPresentQueueIndex())
	{
		CreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		CreateInfo.queueFamilyIndexCount = 2;
		CreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		CreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}

	CreateInfo.preTransform = SurfaceCapabilities.currentTransform;
	CreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	CreateInfo.presentMode = PresentMode;

	Swapchain = VulkanContext::Get()->GetDevice().createSwapchainKHRUnique(CreateInfo);
	SwapchainImages = VulkanContext::Get()->GetDevice().getSwapchainImagesKHR(Swapchain.get());
}

vk::SurfaceFormatKHR VulkanSwapchain::ChooseSwapchainFormat()
{
	std::vector<vk::SurfaceFormatKHR> SurfaceFormats = VulkanContext::Get()->GetPhysicalDevice().getSurfaceFormatsKHR(VulkanContext::Get()->GetSurface());
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
		if (SurfaceFormats[0].format == vk::Format::eUndefined)
		{
			//Free to choose our own format and color space
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
	std::vector<vk::PresentModeKHR> SurfacePresentModes = VulkanContext::Get()->GetPhysicalDevice().getSurfacePresentModesKHR(VulkanContext::Get()->GetSurface());

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

void VulkanSwapchain::CreateImageViews()
{
	SwapchainImageViews.clear();

	for (size_t i=0; i < SwapchainImages.size(); ++i)
	{
		vk::ImageViewCreateInfo CreateInfo;

		//Basic Info
		CreateInfo.image = SwapchainImages[i];
		CreateInfo.viewType = vk::ImageViewType::e2D;
		CreateInfo.format = SwapchainImageFormat;

		//Channel Mapping
		CreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
		CreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
		CreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
		CreateInfo.components.a = vk::ComponentSwizzle::eIdentity;

		//Purpose of this image
		CreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		CreateInfo.subresourceRange.baseMipLevel = 0;
		CreateInfo.subresourceRange.levelCount = 1;
		CreateInfo.subresourceRange.baseArrayLayer = 0;
		CreateInfo.subresourceRange.layerCount = 1;

		SwapchainImageViews.push_back(VulkanContext::Get()->GetDevice().createImageViewUnique(CreateInfo));
	}
}

void VulkanSwapchain::CreateDepthBuffer()
{
	vk::Device Device = VulkanContext::Get()->GetDevice();

	DepthFormat = VulkanContext::Get()->FindSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
         vk::ImageTiling::eOptimal,
         vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	
	bool HasStencil = (DepthFormat == vk::Format::eD32SfloatS8Uint) || (DepthFormat == vk::Format::eD24UnormS8Uint);

	vk::ImageCreateInfo ImageInfo;
	ImageInfo.imageType = vk::ImageType::e2D;
	ImageInfo.format = DepthFormat;
	ImageInfo.extent.width = SwapchainExtent.width;
	ImageInfo.extent.height = SwapchainExtent.height;
	ImageInfo.extent.depth = 1;
	ImageInfo.mipLevels = 1;
	ImageInfo.arrayLayers = 1;
	ImageInfo.samples = vk::SampleCountFlagBits::e1;
	ImageInfo.initialLayout = vk::ImageLayout::eUndefined;
	ImageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
	ImageInfo.sharingMode = vk::SharingMode::eExclusive;

	DepthBuffer = Device.createImageUnique(ImageInfo);

	vk::MemoryRequirements MemReqs = Device.getImageMemoryRequirements(DepthBuffer.get());

	vk::MemoryAllocateInfo AllocInfo;
	AllocInfo.allocationSize = MemReqs.size;
	AllocInfo.memoryTypeIndex = VulkanContext::FindMemoryType(MemReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

	DepthBufferMemory = Device.allocateMemoryUnique(AllocInfo);

	Device.bindImageMemory(DepthBuffer.get(), DepthBufferMemory.get(), 0);

	vk::ImageViewCreateInfo ViewInfo;
	ViewInfo.image = DepthBuffer.get();
	ViewInfo.format = DepthFormat;
	ViewInfo.subresourceRange.levelCount = 1;
	ViewInfo.subresourceRange.layerCount = 1;
	ViewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
	
	if (HasStencil)
	{
		ViewInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
	}

	ViewInfo.viewType = vk::ImageViewType::e2D;

	DepthBufferView = Device.createImageViewUnique(ViewInfo);
}