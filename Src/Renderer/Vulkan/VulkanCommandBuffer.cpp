#include "VulkanCommandBuffer.h"

#include "VulkanRenderer.h"

VulkanCommandBuffer::VulkanCommandBuffer()
{
	vk::CommandBufferAllocateInfo AllocInfo;
	AllocInfo.commandPool = VulkanRenderer::Get()->GetCommandPool();
	AllocInfo.level = vk::CommandBufferLevel::ePrimary;
	AllocInfo.commandBufferCount = 1;

	CommandBuffer = VulkanRenderer::Get()->GetDevice().allocateCommandBuffers(AllocInfo).front();
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