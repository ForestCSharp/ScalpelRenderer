#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <vector>

//TODO: Move this. Buffer should only know about its data, shouldn't care about contents
//NOTE see SpirV_Reflect code in VulkanGraphicsPipeline
struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

//Type of Buffer to create
enum class EBufferType
{
	VertexBuffer,
	IndexBuffer
};

class VulkanBufferUtils 
{
public:
	static void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::UniqueBuffer& OutBuffer, vk::UniqueDeviceMemory& OutMemory);
	static void CopyBuffer(vk::UniqueBuffer& SourceBuffer, vk::UniqueBuffer& DestinationBuffer, vk::DeviceSize CopySize);
};

class VulkanBuffer
{
public:
	VulkanBuffer(void* Data, vk::DeviceSize DataSize, EBufferType BufferType);
	const vk::Buffer GetHandle() { return Buffer.get(); }

protected:

	vk::UniqueBuffer Buffer;
	vk::UniqueDeviceMemory Memory;
};