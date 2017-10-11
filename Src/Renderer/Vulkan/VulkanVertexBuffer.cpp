#include "VulkanVertexBuffer.h"

#include "VulkanContext.h"

VulkanVertexBuffer::VulkanVertexBuffer()
{
	//TODO: Take in vertices as argument
	vk::DeviceSize BufferSize = sizeof(vertices[0]) * vertices.size();
	
	//Staging Buffer
	vk::UniqueBuffer StagingBuffer;
	vk::UniqueDeviceMemory StagingMemory;
	CreateBuffer(BufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, StagingBuffer, StagingMemory);

	vk::Device Device = VulkanContext::Get()->GetDevice();

	void* data = Device.mapMemory(StagingMemory.get(), 0, BufferSize);
	memcpy(data, vertices.data(), (size_t) BufferSize);
	Device.unmapMemory(StagingMemory.get());

	CreateBuffer(BufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, Buffer, Memory);

	CopyBuffer(StagingBuffer, Buffer, BufferSize);
}

void VulkanVertexBuffer::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::UniqueBuffer& OutBuffer, vk::UniqueDeviceMemory& OutMemory)
{
	vk::Device Device = VulkanContext::Get()->GetDevice();
	
	vk::BufferCreateInfo CreateInfo;
	CreateInfo.size = size;
	CreateInfo.usage = usage;
	CreateInfo.sharingMode = vk::SharingMode::eExclusive;

	OutBuffer = Device.createBufferUnique(CreateInfo);

	vk::MemoryRequirements MemoryRequirements = Device.getBufferMemoryRequirements(OutBuffer.get());
	
	vk::MemoryAllocateInfo AllocInfo;
	AllocInfo.allocationSize = MemoryRequirements.size;
	AllocInfo.memoryTypeIndex = VulkanContext::FindMemoryType(MemoryRequirements.memoryTypeBits, properties);

	OutMemory = Device.allocateMemoryUnique(AllocInfo);

	Device.bindBufferMemory(OutBuffer.get(), OutMemory.get(), 0);
}

void VulkanVertexBuffer::CopyBuffer(vk::UniqueBuffer& SourceBuffer, vk::UniqueBuffer& DestinationBuffer, vk::DeviceSize CopySize)
{
	vk::CommandBufferAllocateInfo AllocInfo;
	AllocInfo.level = vk::CommandBufferLevel::ePrimary;
	AllocInfo.commandPool = VulkanContext::Get()->GetCommandPool();
	AllocInfo.commandBufferCount = 1;

	std::vector<vk::UniqueCommandBuffer> CommandBuffers = VulkanContext::Get()->GetDevice().allocateCommandBuffersUnique(AllocInfo);
	vk::UniqueCommandBuffer& CommandBuffer = CommandBuffers[0];
	
	vk::CommandBufferBeginInfo BeginInfo;
	BeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	
	CommandBuffer->begin(BeginInfo);
	vk::BufferCopy CopyRegion;
	CopyRegion.size = CopySize;
	CommandBuffer->copyBuffer(SourceBuffer.get(), DestinationBuffer.get(), 1, &CopyRegion);
	CommandBuffer->end();

	vk::SubmitInfo SubmitInfo;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &(CommandBuffer.get());
	
	VulkanContext::Get()->GetGraphicsQueue().submit(1, &SubmitInfo, vk::Fence());
	VulkanContext::Get()->GetGraphicsQueue().waitIdle();
}