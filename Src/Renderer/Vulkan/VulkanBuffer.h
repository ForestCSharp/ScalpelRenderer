#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>

//TODO: Move this. Buffer should only know about its data, shouldn't care about contents
struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static std::vector<vk::VertexInputBindingDescription> GetBindingDescriptions()
	{
		std::vector<vk::VertexInputBindingDescription> Descriptions;
		Descriptions.resize(1);

		Descriptions[0].binding = 0;
		Descriptions[0].stride = sizeof(Vertex);
		Descriptions[0].inputRate = vk::VertexInputRate::eVertex;

		return Descriptions;
	}

	static std::vector<vk::VertexInputAttributeDescription> GetAttributeDescriptions()
	{
		std::vector<vk::VertexInputAttributeDescription> Descriptions;		
		Descriptions.resize(3);

		Descriptions[0].binding = 0;
		Descriptions[0].location = 0;
		Descriptions[0].format = vk::Format::eR32G32Sfloat;
		Descriptions[0].offset = offsetof(Vertex, pos);

		Descriptions[1].binding = 0;
		Descriptions[1].location = 1;
		Descriptions[1].format = vk::Format::eR32G32B32Sfloat;
		Descriptions[1].offset = offsetof(Vertex, color);
		
		Descriptions[2].binding = 0;
        Descriptions[2].location = 2;
        Descriptions[2].format = vk::Format::eR32G32Sfloat;
        Descriptions[2].offset = offsetof(Vertex, texCoord);
		return Descriptions;
	}
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

//Type of Buffer to create
enum class EBufferType
{
	VertexBuffer,
	IndexBuffer
};

//TODO: Move to own file
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