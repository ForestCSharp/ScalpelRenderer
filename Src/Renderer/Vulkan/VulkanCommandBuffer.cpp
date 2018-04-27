#include "VulkanCommandBuffer.h"

#include "VulkanContext.h"

VulkanCommandBuffer::VulkanCommandBuffer(bool bSecondary)
{
	vk::CommandBufferAllocateInfo AllocInfo;
	AllocInfo.commandPool = VulkanContext::Get()->GetCommandPool();
	AllocInfo.level = bSecondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;
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

void VulkanCommandBuffer::BeginSecondary(vk::CommandBufferUsageFlags Flags, vk::RenderPass& RenderPass)
{
	vk::CommandBufferInheritanceInfo InheritanceInfo;
	InheritanceInfo.renderPass = RenderPass;
	InheritanceInfo.subpass = 0; //TODO: handle subpass when we support them
	//InheritanceInfo.framebuffer = VK_NULL_HANDLE;
	//TODO: Occlusion Query Support

	vk::CommandBufferBeginInfo BeginInfo;
	BeginInfo.flags = Flags;
	BeginInfo.pInheritanceInfo = &InheritanceInfo;
	
	CommandBuffer.begin(BeginInfo);


	/*TODO: Note From Spec: Specifying the exact framebuffer that the secondary command buffer will be 
	 		executed with may result in better performance at command buffer execution time */


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