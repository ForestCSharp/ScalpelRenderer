#pragma once

#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

class VulkanGraphicsPipeline
{
public:

	VulkanGraphicsPipeline();
	~VulkanGraphicsPipeline();
	
	void BuildPipeline(class VulkanRenderPass& RenderPass,const std::string& VertexShader, const std::string& FragmentShader);
	vk::Pipeline GetHandle() { return GraphicsPipeline.get(); }
	vk::PipelineLayout GetLayout() { return PipelineLayout.get(); }

protected: //Internal Pipeline Member variable

	vk::UniquePipeline GraphicsPipeline;

public: //Shader Stage Functions

	std::vector<char> LoadShaderFromFile(const std::string& filename);
	vk::ShaderModule  CreateShaderModule(std::vector<char> spvCode);

public://Fixed Function Pipeline State

	vk::PipelineVertexInputStateCreateInfo VertexInput;
	
	void SetVertexInputBindings(std::vector<vk::VertexInputBindingDescription>& InputBindings, std::vector<vk::VertexInputAttributeDescription>& AttributeBindings);
	std::vector<vk::VertexInputBindingDescription> VertexInputBindings;
	std::vector<vk::VertexInputAttributeDescription> VertexAttributeBindings;

	vk::PipelineInputAssemblyStateCreateInfo InputAssembly;

	vk::PipelineTessellationStateCreateInfo Tessellation;
	
	vk::PipelineViewportStateCreateInfo ViewportState;
	vk::Viewport Viewport;
	vk::Rect2D Scissor;

	vk::PipelineRasterizationStateCreateInfo Rasterizer;

	vk::PipelineMultisampleStateCreateInfo Multisampling;

	vk::PipelineDepthStencilStateCreateInfo DepthStencil;

	vk::PipelineColorBlendStateCreateInfo ColorBlending;
	vk::PipelineColorBlendAttachmentState ColorBlendAttachment;

	vk::PipelineDynamicStateCreateInfo DynamicState;
	std::vector<vk::DynamicState> DynamicStates;

	vk::UniqueDescriptorSetLayout DescriptorSetLayout;
	std::vector<vk::DescriptorSetLayoutBinding> DescriptorBindings;
	vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo;
	vk::UniquePipelineLayout PipelineLayout;
};