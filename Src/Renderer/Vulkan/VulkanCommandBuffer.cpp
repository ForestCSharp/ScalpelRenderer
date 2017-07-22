#include "VulkanCommandBuffer.h"

#include "VulkanContext.h"

VulkanCommandBuffer::VulkanCommandBuffer()
{
	vk::CommandBufferAllocateInfo AllocInfo;
	AllocInfo.commandPool = VulkanContext::Get()->GetCommandPool();
	AllocInfo.level = vk::CommandBufferLevel::ePrimary;
	AllocInfo.commandBufferCount = 1;

	CommandBuffer = VulkanContext::Get()->GetDevice().allocateCommandBuffers(AllocInfo).front();
}

void VulkanCommandBuffer::Begin()
{
	vk::CommandBufferBeginInfo BeginInfo;
	CommandBuffer.begin(BeginInfo);
}

void VulkanCommandBuffer::End()
{
	CommandBuffer.end();
}