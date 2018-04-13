#pragma once

#include <vulkan/vulkan.hpp>

class VulkanImage
{
public:
    //Load in a texture from file
    VulkanImage(class std::string& filename);
    //Creates a general purpose image and doesn't fill it with data
    VulkanImage(uint32_t Width, uint32_t Height, vk::Format Format, vk::ImageTiling Tiling,
                vk::ImageUsageFlags Usage, vk::MemoryPropertyFlags MemoryProperties);
    
    void LoadImageFromFile(class std::string& filename);

    void CreateImage(uint32_t Width, uint32_t Height, vk::Format Format, vk::ImageTiling Tiling,
                            vk::ImageUsageFlags Usage, vk::MemoryPropertyFlags MemoryProperties);

    void TransitionImageLayout(vk::ImageLayout TargetLayout);

    void CopyBufferToImage(vk::Buffer Buffer, uint32_t width, uint32_t height);

    //Optional Image View
    vk::ImageView GetImageView();
    void CreateImageView();

    //Optional Sampler
    vk::Sampler GetSampler();
    void CreateSampler();

    //Optional Descriptor Info
    vk::DescriptorImageInfo GetDescriptorInfo();
    void CreateDescriptorInfo();

    vk::Format GetFormat() { return ImageFormat; }

protected:

    vk::UniqueImage Image;
    vk::Format ImageFormat;
    vk::ImageLayout ImageLayout;
    vk::UniqueDeviceMemory ImageMemory;

    //Optional Image View
    vk::UniqueImageView ImageView;
    bool bImageViewBuilt = false;

    //Optional Sampler
    vk::UniqueSampler ImageSampler;
    bool bSamplerBuilt = false;

    //Optional Descritor Info
    vk::DescriptorImageInfo DescriptorImageInfo;
    bool bDescriptorInfoBuilt = false;
};