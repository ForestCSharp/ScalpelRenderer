#pragma once

#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanGraphicsPipeline.h"

#include <map>

//Represents a Renderable Entity (static/skinned meshes, full-screen quad, sprites)
class RenderItem
{
public:
    RenderItem(void* VertexData, vk::DeviceSize VertexDataSize, void* IndexData, vk::DeviceSize IndexDataSize, uint32_t NumIndices) : 
        VertexBuffer(VertexData, VertexDataSize, EBufferType::VertexBuffer),
        IndexBuffer(IndexData, IndexDataSize, EBufferType::IndexBuffer),
        IndexCount(NumIndices)
    {}

    //Takes in a command buffer and adds the necessary binds and draw calls for this render item
    void AddCommands(VulkanCommandBuffer& CommandBuffer, VulkanGraphicsPipeline* Pipeline, std::vector<vk::WriteDescriptorSet> TEST_DESCRIPTOR_WRITES) //TODO: Remove temp descriptor write
    {
        vk::Buffer VertexBuffers[] = {VertexBuffer.GetHandle()};
        vk::DeviceSize Offsets[] = {0};

        //Create Descriptor Set for this pipeline if we don't yet have one for this render item
        auto& FoundDescriptorData = PipelineDescriptors.find(Pipeline);
        if (FoundDescriptorData == PipelineDescriptors.end())
        {
            //TODO: Potential race condition when building up Command buffers in parallel
            FoundDescriptorData = PipelineDescriptors.emplace(Pipeline, Pipeline->AllocateDescriptorSets(1)).first;
            
            //TODO: Need way to update descriptor set / fill it with meaningful data (currently hard coded for testing)
            auto& DescriptorSet = FoundDescriptorData->second.Sets[0].get();
            TEST_DESCRIPTOR_WRITES[0].dstSet = DescriptorSet;
            TEST_DESCRIPTOR_WRITES[1].dstSet = DescriptorSet;
            VulkanContext::Get()->GetDevice().updateDescriptorSets(TEST_DESCRIPTOR_WRITES, 0);
        }
        auto& DescriptorSet = FoundDescriptorData->second.Sets[0].get();

        //[1] Bind Descriptor Set
        CommandBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Pipeline->GetLayout(), 0, 1, &DescriptorSet, 0, nullptr);
        //[2] Bind Vertex Buffer
        CommandBuffer().bindVertexBuffers(0, 1, VertexBuffers, Offsets);
        //[3] Bind Index Buffer
        CommandBuffer().bindIndexBuffer(IndexBuffer.GetHandle(), 0, vk::IndexType::eUint32);
        //[4] DrawIndexed
        CommandBuffer().drawIndexed(IndexCount, 1, 0, 0, 0);
    }

    VulkanBuffer VertexBuffer;
    VulkanBuffer IndexBuffer;
    uint32_t     IndexCount;
    
    //TODO: Definition of Vertex input for matching to pipeline
    //TODO: OR just a std::set of Pipeline Pointers pertaining to this particular RenderItem

    std::map<VulkanGraphicsPipeline*, DescriptorData> PipelineDescriptors;
};