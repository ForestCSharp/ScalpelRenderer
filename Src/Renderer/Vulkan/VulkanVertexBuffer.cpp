#include "VulkanVertexBuffer.h"

#include "VulkanContext.h"

VulkanVertexBuffer::VulkanVertexBuffer()
{
	vk::BufferCreateInfo CreateInfo;

	vk::Device Device = VulkanContext::Get()->GetDevice();

	//TODO: Have CTOR pass in vertices
	CreateInfo.size = sizeof(vertices[0]) * vertices.size();
	CreateInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
	CreateInfo.sharingMode = vk::SharingMode::eExclusive;

	Buffer = Device.createBufferUnique(CreateInfo);

	vk::MemoryRequirements MemoryRequirements = Device.getBufferMemoryRequirements(Buffer.get());

	vk::MemoryAllocateInfo AllocInfo;
	AllocInfo.allocationSize = MemoryRequirements.size;
	AllocInfo.memoryTypeIndex = FindMemoryType(MemoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	Memory = Device.allocateMemoryUnique(AllocInfo);

	Device.bindBufferMemory(Buffer.get(), Memory.get(), 0);

	void* data = Device.mapMemory(Memory.get(), 0, CreateInfo.size);
	memcpy(data, vertices.data(), (size_t) CreateInfo.size);
	Device.unmapMemory(Memory.get());
}

//TODO: Bubble this up to generic VulkanBuffer base class?
uint32_t VulkanVertexBuffer::FindMemoryType(uint32_t TypeFilter, vk::MemoryPropertyFlags Properties)
{
	vk::PhysicalDeviceMemoryProperties MemoryProperties = VulkanContext::Get()->GetPhysicalDevice().getMemoryProperties();
	for (uint32_t i = 0; i < MemoryProperties.memoryTypeCount; i++) 
	{
		if ((TypeFilter & (1 << i)) && (MemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties) 
		{
			return i;
		}
	}

	throw std::runtime_error("VertexBuffer: Failed to find suitable memory type");
}