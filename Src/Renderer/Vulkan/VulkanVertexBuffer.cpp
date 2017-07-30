#include "VulkanVertexBuffer.h"

#include "VulkanContext.h"

VulkanVertexBuffer::VulkanVertexBuffer()
{
	vk::BufferCreateInfo CreateInfo;

	//TODO: Have CTOR pass in vertices
	CreateInfo.size = sizeof(vertices[0]) * vertices.size();
	CreateInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
	CreateInfo.sharingMode = vk::SharingMode::eExclusive;

	Buffer = VulkanContext::Get()->GetDevice().createBufferUnique(CreateInfo);
}