#include "VulkanUniform.h"
#include "VulkanContext.h"

VulkanUniform::VulkanUniform(vk::DeviceSize BufferSize)
{
    VulkanBufferUtils::CreateBuffer(BufferSize, vk::BufferUsageFlagBits::eUniformBuffer, 
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 
        UniformBuffer, UniformMemory);   
        
    DescriptorInfo.buffer = UniformBuffer.get();
    DescriptorInfo.offset = 0;
    DescriptorInfo.range = BufferSize;
}

void VulkanUniform::UpdateUniformData(void* Data, vk::DeviceSize DataSize)
{
    vk::Device Device = VulkanContext::Get()->GetDevice();

    void* MappedMemory = Device.mapMemory(UniformMemory.get(), 0, DataSize);
	memcpy(MappedMemory, Data, (size_t) DataSize);
	Device.unmapMemory(UniformMemory.get());
}

const vk::DescriptorBufferInfo VulkanUniform::GetDescriptorInfo() const
{
    return DescriptorInfo;
}