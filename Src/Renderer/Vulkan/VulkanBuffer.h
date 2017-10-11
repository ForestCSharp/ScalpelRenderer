#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>

//TODO: Generic parent class with pure virtual functions for binding and attribute descriptions?
struct Vertex
{
	glm::vec2 pos;
	glm::vec3 color;

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
		Descriptions.resize(2);

		Descriptions[0].binding = 0;
		Descriptions[0].location = 0;
		Descriptions[0].format = vk::Format::eR32G32Sfloat;
		Descriptions[0].offset = offsetof(Vertex, pos);

		Descriptions[1].binding = 0;
		Descriptions[1].location = 1;
		Descriptions[1].format = vk::Format::eR32G32B32Sfloat;
		Descriptions[1].offset = offsetof(Vertex, color);

		return Descriptions;
	}
};

const std::vector<Vertex> vertices = 
{
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

class VulkanBuffer
{
public:

	VulkanBuffer();
	const vk::Buffer GetHandle() { return Buffer.get(); }

protected:

	void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::UniqueBuffer& OutBuffer, vk::UniqueDeviceMemory& OutMemory);
	void CopyBuffer(vk::UniqueBuffer& SourceBuffer, vk::UniqueBuffer& DestinationBuffer, vk::DeviceSize CopySize);
	uint32_t FindMemoryType(uint32_t TypeFilter, vk::MemoryPropertyFlags Properties);

protected:

	vk::UniqueBuffer Buffer;
	vk::UniqueDeviceMemory Memory;
};