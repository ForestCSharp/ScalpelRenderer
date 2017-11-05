#pragma once

#include <vulkan/vulkan.hpp>

//A Primary Command Buffer
class VulkanCommandBuffer
{
public:

	VulkanCommandBuffer();
	void Begin(); //Default with eSimultaneousUse bit
	void Begin(vk::CommandBufferUsageFlags Flags);
	void End();

	//Submits and command buffer to the graphics queue and waits
	void SubmitWaitIdle();

	//Gets internal command buffer object
	vk::CommandBuffer Get() {return CommandBuffer;}
	vk::CommandBuffer operator()() {return CommandBuffer;}

protected:

	vk::CommandBuffer CommandBuffer;

};