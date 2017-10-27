#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "VulkanBuffer.h"

class VulkanUniform
{
public:

    VulkanUniform(vk::DeviceSize BufferSize);
    void UpdateUniformData(void* Data, vk::DeviceSize DataSize);
    const vk::DescriptorBufferInfo GetDescriptorInfo() const;

protected:

    vk::UniqueBuffer UniformBuffer;
    vk::UniqueDeviceMemory UniformMemory;
    vk::DescriptorBufferInfo DescriptorInfo;
};