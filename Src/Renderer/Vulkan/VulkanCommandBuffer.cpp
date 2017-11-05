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
	Begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
}

void VulkanCommandBuffer::Begin(vk::CommandBufferUsageFlags Flags)
{
	vk::CommandBufferBeginInfo BeginInfo;
	BeginInfo.flags = Flags;
	CommandBuffer.begin(BeginInfo);
}

void VulkanCommandBuffer::End()
{
	CommandBuffer.end();
}

void VulkanCommandBuffer::SubmitWaitIdle()
{
	vk::SubmitInfo SubmitInfo;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffer;
	
	VulkanContext::Get()->GetGraphicsQueue().submit(1, &SubmitInfo, vk::Fence());
	VulkanContext::Get()->GetGraphicsQueue().waitIdle();
}