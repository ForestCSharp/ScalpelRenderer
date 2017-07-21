#pragma once

#include <vulkan/vulkan.hpp>

//A Primary Command Buffer
class VulkanCommandBuffer
{
public:

	VulkanCommandBuffer();
	void Begin();
	void End();

	//Gets internal command buffer object
	vk::CommandBuffer Get() {return CommandBuffer;}
	vk::CommandBuffer operator()() {return CommandBuffer;}

protected:

	vk::CommandBuffer CommandBuffer;

};