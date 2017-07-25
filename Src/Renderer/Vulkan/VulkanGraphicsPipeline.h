#pragma once

#include <vulkan/vulkan.hpp>

class VulkanGraphicsPipeline
{
public:

	VulkanGraphicsPipeline();
	
	void BuildPipeline();

protected: //Internal Pipeline Member variable

	vk::Pipeline GraphicsPipeline;
	vk::GraphicsPipelineCreateInfo CreateInfo;

public: //Shader Stage Functions



protected: //Shader Stages

	vk::ShaderModule VertexShaderModule;
	vk::ShaderModule FragmentShaderModule;

public://Fixed Function Pipeline State

	vk::PipelineVertexInputStateCreateInfo VertexInput;

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

	vk::PipelineLayout PipelineLayout;
};