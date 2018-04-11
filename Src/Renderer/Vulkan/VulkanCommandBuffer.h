#pragma once

#include <vulkan/vulkan.hpp>

//A Primary Command Buffer
class VulkanCommandBuffer
{
public:

	VulkanCommandBuffer(bool bSecondary = false);
	void Begin(); //Default with eSimultaneousUse bit
	void Begin(vk::CommandBufferUsageFlags Flags);
	void BeginSecondary(vk::CommandBufferUsageFlags Flags, vk::RenderPass& RenderPass);
	void End();

	//Submits and command buffer to the graphics queue and waits
	void SubmitWaitIdle();

	//Gets internal command buffer object
	vk::CommandBuffer GetHandle() {return CommandBuffer;}
	vk::CommandBuffer operator()() {return CommandBuffer;}

protected:

	vk::CommandBuffer CommandBuffer;

};