#include "../Vulkan/VulkanBuffer.h"
#include "../Vulkan/VulkanCommandBuffer.h"

//Represents a Renderable Entity (static/skinned meshes, full-screen quad, sprites)
class RenderItem
{
public:
    RenderItem(void* VertexData, vk::DeviceSize VertexDataSize, void* IndexData, vk::DeviceSize IndexDataSize, uint32_t NumIndices) : 
        VertexBuffer(VertexData, VertexDataSize, EBufferType::VertexBuffer),
        IndexBuffer(IndexData, IndexDataSize, EBufferType::IndexBuffer),
        IndexCount(NumIndices)
    {

    }

    //Takes in a command buffer and performs the necessary binds and draw calls for this render item
    void BuildCommands(VulkanCommandBuffer& CommandBuffer, VulkanGraphicsPipeline& Pipeline, vk::DescriptorSet& DescriptorSet)
    {
        vk::Buffer VertexBuffers[] = {VertexBuffer.GetHandle()};
        vk::DeviceSize Offsets[] = {0};

        //TODO: This can probably live outside of our RenderItem's build commands
        CommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Pipeline.GetLayout(), 0, 1, &DescriptorSet, 0, nullptr);
        
        CommandBuffer().bindVertexBuffers(0, 1, VertexBuffers, Offsets);
        //[3] Bind Index Buffer
        CommandBuffer().bindIndexBuffer(IndexBuffer.GetHandle(), 0, vk::IndexType::eUint16); //TODO: Support Multiple Index Types
        //[4] DrawIndexed
        CommandBuffer().drawIndexed(IndexCount, 1, 0, 0, 0);
    }

    VulkanBuffer VertexBuffer;
    VulkanBuffer IndexBuffer;
    uint32_t     IndexCount;
};