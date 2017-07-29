#include "VulkanRenderPass.h"

#include "VulkanContext.h"
#include "VulkanSwapchain.h"

VulkanRenderPass::VulkanRenderPass()
{

}

void VulkanRenderPass::BuildRenderPass(VulkanSwapchain& Swapchain)
{
	//TODO: Make generic
	//TODO: Make config structs member vars
	//TODO: way to build up attachments dynamically

	vk::AttachmentDescription ColorAttachment;
	
	ColorAttachment.format = Swapchain.GetFormat();
	ColorAttachment.samples = vk::SampleCountFlagBits::e1;
	ColorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	ColorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	ColorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	ColorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	ColorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	ColorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
	
	//Color attachment Reference for our subpass
	vk::AttachmentReference ColorAttachmentRef;
	ColorAttachmentRef.attachment = 0;
	ColorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	//Our subpass
	vk::SubpassDescription Subpass;
	Subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	Subpass.colorAttachmentCount = 1;
	Subpass.pColorAttachments = &ColorAttachmentRef;

	//The Renderpass itself
	vk::RenderPassCreateInfo CreateInfo;
	CreateInfo.attachmentCount = 1;
	CreateInfo.pAttachments = &ColorAttachment;
	CreateInfo.subpassCount = 1;
	CreateInfo.pSubpasses = &Subpass;

	//Subpass Dependency
	vk::SubpassDependency Dependency;
	Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	Dependency.dstSubpass = 0;
	Dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	//Dependency.srcAccessMask = 0;
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
		vk::ImageView attachments[] = {UniqueImageView.get()};

		vk::FramebufferCreateInfo FramebufferCreateInfo;
		FramebufferCreateInfo.renderPass = GetRenderPass();
		FramebufferCreateInfo.attachmentCount = 1;
		FramebufferCreateInfo.pAttachments = attachments;
		FramebufferCreateInfo.width = Swapchain.GetExtent().width;
		FramebufferCreateInfo.height = Swapchain.GetExtent().height;
		FramebufferCreateInfo.layers = 1;

		Framebuffers.push_back(VulkanContext::Get()->GetDevice().createFramebufferUnique(FramebufferCreateInfo));
	}
	//End Framebuffer Creation
}