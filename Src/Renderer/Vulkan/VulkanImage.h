#pragma once

#include <vulkan/vulkan.hpp>
#include <string>

class VulkanImage
{
public:
    //Load in a texture from file
    VulkanImage(std::string& filename);
    //Creates a general purpose image and doesn't fill it with data
    VulkanImage(uint32_t Width, uint32_t Height, vk::Format Format, vk::ImageTiling Tiling,
                vk::ImageUsageFlags Usage, vk::MemoryPropertyFlags MemoryProperties);
    
    void LoadImageFromFile(std::string& filename);

    void CreateImage(uint32_t Width, uint32_t Height, vk::Format Format, vk::ImageTiling Tiling,
                            vk::ImageUsageFlags Usage, vk::MemoryPropertyFlags MemoryProperties);

    void TransitionImageLayout(vk::ImageLayout TargetLayout);

    void CopyBufferToImage(vk::Buffer Buffer, uint32_t width, uint32_t height);

protected:

    vk::UniqueImage Image;
    vk::Format ImageFormat;
    vk::ImageLayout ImageLayout;
    vk::UniqueDeviceMemory ImageMemory;
};