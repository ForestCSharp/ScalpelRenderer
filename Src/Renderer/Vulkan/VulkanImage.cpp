#include "VulkanImage.h"

#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>

VulkanImage::VulkanImage(std::string& filename)
{
    LoadImageFromFile(filename);
}

VulkanImage::VulkanImage(uint32_t Width, uint32_t Height, vk::Format Format, vk::ImageTiling Tiling,
                            vk::ImageUsageFlags Usage, vk::MemoryPropertyFlags MemoryProperties)
{
    CreateImage(Width, Height, Format, Tiling, Usage, MemoryProperties);
}

void VulkanImage::LoadImageFromFile(std::string& filename)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* PixelData = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize ImageSize = texWidth * texHeight * 4;

    if (!PixelData) 
    {
        std::cout << "Failed to load image data" << std::endl;
        throw std::runtime_error("failed to load texture image!");
    }

    vk::UniqueBuffer StagingBuffer;
    vk::UniqueDeviceMemory StagingMemory;

    VulkanBufferUtils::CreateBuffer(ImageSize, vk::BufferUsageFlagBits::eTransferSrc, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
        StagingBuffer, StagingMemory);

    vk::Device Device = VulkanContext::Get()->GetDevice();

    void* MappedMemory = Device.mapMemory(StagingMemory.get(), 0, ImageSize);
        memcpy(MappedMemory, PixelData, static_cast<size_t>(ImageSize));
    Device.unmapMemory(StagingMemory.get());

    stbi_image_free(PixelData);

    CreateImage(texWidth, texHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
    vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);

    TransitionImageLayout(vk::ImageLayout::eTransferDstOptimal);
    CopyBufferToImage(StagingBuffer.get(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
}

void VulkanImage::CreateImage(uint32_t Width, uint32_t Height, vk::Format Format, vk::ImageTiling Tiling,
                                vk::ImageUsageFlags Usage, vk::MemoryPropertyFlags MemoryProperties)
{
    vk::Device Device = VulkanContext::Get()->GetDevice();

    vk::ImageCreateInfo ImageCreateInfo;
    ImageCreateInfo.imageType = vk::ImageType::e2D;
    ImageCreateInfo.extent.width = Width;
    ImageCreateInfo.extent.height = Height;
    ImageCreateInfo.extent.depth = 1;
    ImageCreateInfo.mipLevels = 1;
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.format = Format;
    ImageCreateInfo.tiling = Tiling;
    ImageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
    ImageCreateInfo.usage = Usage;
    ImageCreateInfo.samples = vk::SampleCountFlagBits::e1;
    ImageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    
    ImageLayout = ImageCreateInfo.initialLayout;
    ImageFormat = Format;
    Image = Device.createImageUnique(ImageCreateInfo, nullptr); 
    
    vk::MemoryRequirements MemoryRequirements = Device.getImageMemoryRequirements(Image.get());
    vk::MemoryAllocateInfo MemoryAllocateInfo;
    MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
    MemoryAllocateInfo.memoryTypeIndex = VulkanContext::FindMemoryType(MemoryRequirements.memoryTypeBits, MemoryProperties);

    ImageMemory = Device.allocateMemoryUnique(MemoryAllocateInfo, nullptr);
    Device.bindImageMemory(Image.get(), ImageMemory.get(), 0);
}

void VulkanImage::TransitionImageLayout(vk::ImageLayout TargetLayout)
{
    VulkanCommandBuffer CommandBuffer;
    CommandBuffer.Begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    vk::ImageMemoryBarrier Barrier;
    Barrier.oldLayout = ImageLayout;
    Barrier.newLayout = TargetLayout;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.image = Image.get();
    Barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.levelCount = 1;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;
    
    vk::PipelineStageFlags SrcStage;
    vk::PipelineStageFlags DstStage;

    //TODO: Better way to handle this?
    if (ImageLayout == vk::ImageLayout::eUndefined && TargetLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        Barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        SrcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        DstStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (ImageLayout == vk::ImageLayout::eTransferDstOptimal && TargetLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        Barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        Barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        SrcStage = vk::PipelineStageFlagBits::eTransfer;
        DstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        std::cout << "Unsupported Image Layout Transition" << std::endl;
    }

    vk::DependencyFlags DependencyFlags;
    CommandBuffer.Get().pipelineBarrier(SrcStage, DstStage, DependencyFlags, nullptr, nullptr, vk::ArrayProxy<const vk::ImageMemoryBarrier>(Barrier));

    CommandBuffer.End();
    CommandBuffer.SubmitWaitIdle();

    ImageLayout = TargetLayout;
}

void VulkanImage::CopyBufferToImage(vk::Buffer Buffer, uint32_t width, uint32_t height)
{
    VulkanCommandBuffer CommandBuffer;
    CommandBuffer.Begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    vk::BufferImageCopy CopyRegion;
    CopyRegion.bufferOffset = 0;
    CopyRegion.bufferRowLength = 0;
    CopyRegion.bufferImageHeight = 0;
    CopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    CopyRegion.imageSubresource.mipLevel = 0;
    CopyRegion.imageSubresource.baseArrayLayer = 0;
    CopyRegion.imageSubresource.layerCount = 1;
    CopyRegion.imageOffset = {0,0,0};
    CopyRegion.imageExtent = {width, height, 1};
    CommandBuffer.Get().copyBufferToImage(Buffer, Image.get(), vk::ImageLayout::eTransferDstOptimal, 1, &CopyRegion);
    CommandBuffer.End();
    CommandBuffer.SubmitWaitIdle();
}