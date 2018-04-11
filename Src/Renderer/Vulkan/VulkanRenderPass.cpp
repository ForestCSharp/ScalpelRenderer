#include "VulkanRenderPass.h"

#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandBuffer.h"

VulkanRenderPass::VulkanRenderPass() : CommandBuffer(true /* bSecondary */)
{

}

void VulkanRenderPass::BuildRenderPass(VulkanSwapchain& Swapchain)
{
	//TODO: Make generic

	vk::AttachmentDescription ColorAttachment;
	ColorAttachment.format = Swapchain.GetSwapchainFormat();
	ColorAttachment.samples = vk::SampleCountFlagBits::e1;
	ColorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	ColorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	ColorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	ColorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	ColorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	ColorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentDescription DepthAttachment;
	DepthAttachment.format = Swapchain.GetDepthFormat();
	DepthAttachment.samples = vk::SampleCountFlagBits::e1;
	DepthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	DepthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
	DepthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	DepthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	DepthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	DepthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	
	//Color attachment Reference for our subpass
	vk::AttachmentReference ColorAttachmentRef;
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	//Depth Reference for subpass
	vk::AttachmentReference DepthAttachmentRef;
	DepthAttachmentRef.attachment = 1;
	DepthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

	//Our subpass
	vk::SubpassDescription Subpass;
	Subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	Subpass.colorAttachmentCount = 1;
	Subpass.pColorAttachments = &ColorAttachmentRef;
	Subpass.pDepthStencilAttachment = &DepthAttachmentRef;

	//Array of our two attachments
	std::vector<vk::AttachmentDescription> Attachments = {ColorAttachment, DepthAttachment}; 

	//The Renderpass itself
	vk::RenderPassCreateInfo CreateInfo;
	CreateInfo.attachmentCount = static_cast<uint32_t>(Attachments.size());
	CreateInfo.pAttachments = Attachments.data();
	CreateInfo.subpassCount = 1;
	CreateInfo.pSubpasses = &Subpass;

	//Subpass Dependency
	vk::SubpassDependency Dependency;
	Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependency.dstSubpass = 0; //Currently only have the one subpass
	Dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	Dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	Dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
	
	CreateInfo.dependencyCount = 1;
	CreateInfo.pDependencies = &Dependency;

	RenderPass = VulkanContext::Get()->GetDevice().createRenderPassUnique(CreateInfo);

	//Framebuffer Creation
	Framebuffers.clear();

	std::vector<vk::UniqueImageView>& ImageViews = Swapchain.GetImageViews();

	for (auto& UniqueImageView : ImageViews)
	{
		std::vector<vk::ImageView> attachments = {UniqueImageView.get(), Swapchain.GetDepthView()};

		vk::FramebufferCreateInfo FramebufferCreateInfo;
		FramebufferCreateInfo.renderPass = GetHandle();
		FramebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		FramebufferCreateInfo.pAttachments = attachments.data();
		FramebufferCreateInfo.width = Swapchain.GetExtent().width;
		FramebufferCreateInfo.height = Swapchain.GetExtent().height;
		FramebufferCreateInfo.layers = 1;

		Framebuffers.push_back(VulkanContext::Get()->GetDevice().createFramebufferUnique(FramebufferCreateInfo));
	}
	//End Framebuffer Creation
}

void VulkanRenderPass::BuildCommands()
{
	vk::CommandBufferUsageFlags UsageFlags = vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eSimultaneousUse; 
	CommandBuffer.BeginSecondary(UsageFlags, GetHandle());

	// Have RenderItems sorted by pipeline type
	// [1] For each Pipeline Type, 
	    // [2]vkCmdBindPipeline
		// [3] For each render item of this pipeline type
			// [4] RenderItem.BuildCommands()
		 

	CommandBuffer.End();
}