#pragma once

#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanGraphicsPipeline.h"

#include <map>

//Represents a Renderable Entity (static/skinned meshes, full-screen quad, sprites)
class VulkanRenderItem
{
public:
    VulkanRenderItem(void* VertexData, vk::DeviceSize VertexDataSize, void* IndexData, vk::DeviceSize IndexDataSize, uint32_t NumIndices) : 
        VertexBuffer(VertexData, VertexDataSize, EBufferType::VertexBuffer),
        IndexBuffer(IndexData, IndexDataSize, EBufferType::IndexBuffer),
        IndexCount(NumIndices)
    {}

    //Takes in a command buffer and adds the necessary binds and draw calls for this render item
    void AddCommands(VulkanCommandBuffer& CommandBuffer, VulkanGraphicsPipeline* Pipeline)
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
            UpdateDescriptorSet(DescriptorSet);
            // TEST_DESCRIPTOR_WRITES[0].dstSet = DescriptorSet;
            // TEST_DESCRIPTOR_WRITES[1].dstSet = DescriptorSet;
            // VulkanContext::Get()->GetDevice().updateDescriptorSets(TEST_DESCRIPTOR_WRITES, 0);
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

    void UpdateDescriptorSet(vk::DescriptorSet& DescriptorSet)
    {
        //TODO: Use name to key into binding using pipeline's descriptor info

        std::vector<vk::WriteDescriptorSet> DescriptorWrites;
        for (auto& ImageResource : ImageResources)
        {
            vk::WriteDescriptorSet ImageWrite;          
            ImageWrite.dstSet = DescriptorSet;
            ImageWrite.dstBinding = 1; //TODO: Need to store in map (or fetch by name)
            ImageWrite.dstArrayElement = 0;
            ImageWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
            ImageWrite.descriptorCount = 1;
            ImageWrite.pImageInfo = &ImageResource.second;
            DescriptorWrites.push_back(std::move(ImageWrite));
        }

        for (auto& BufferResource : BufferResources)
        {
            vk::WriteDescriptorSet BufferWrite;
            BufferWrite.dstSet = DescriptorSet;
            BufferWrite.dstBinding = 0; //TODO: Need to store in map
            BufferWrite.dstArrayElement = 0;
            BufferWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
            BufferWrite.descriptorCount = 1;
            BufferWrite.pBufferInfo = &BufferResource.second;
            DescriptorWrites.push_back(std::move(BufferWrite));
        }

        VulkanContext::Get()->GetDevice().updateDescriptorSets(DescriptorWrites, 0);
    }

    VulkanBuffer VertexBuffer;
    VulkanBuffer IndexBuffer;
    uint32_t     IndexCount;
    
    //TODO: Definition of Vertex input for matching to pipeline
    //TODO: OR just a std::set of Pipeline Pointers pertaining to this particular RenderItem

    std::map<VulkanGraphicsPipeline*, DescriptorData> PipelineDescriptors;

    struct cmp_str
    {
        bool operator()(const char* a, const char* b)
        {
            return std::strcmp(a, b) < 0;
        }
    };

    void AddImageResource(const char* Name, vk::DescriptorImageInfo DescriptorImageInfo) 
    {
        ImageResources.emplace(Name, DescriptorImageInfo);
    }

    void AddBufferResource(const char* Name, vk::DescriptorBufferInfo DescriptorBufferInfo)
    {
        BufferResources.emplace(Name, DescriptorBufferInfo);
    }

    std::map<const char *, vk::DescriptorImageInfo, cmp_str>  ImageResources;
    std::map<const char *, vk::DescriptorBufferInfo, cmp_str> BufferResources;
};