#include "VulkanGraphicsPipeline.h"
#include <iostream>
#include <fstream>
#include <algorithm>

#include "VulkanContext.h"
#include "VulkanRenderPass.h"
#include "spirv_reflect.h"

VulkanGraphicsPipeline::VulkanGraphicsPipeline()
{

}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
}

void VulkanGraphicsPipeline::BuildPipeline(VulkanRenderPass& RenderPass, const std::string& VertexShader, const std::string& FragmentShader)
{	
	//TODO: Add additional shader stages

	vk::GraphicsPipelineCreateInfo CreateInfo;

	std::vector<char> VertexSpirV = LoadShaderFromFile(VertexShader);
	vk::ShaderModule VertModule = CreateShaderModule(VertexSpirV);
	vk::PipelineShaderStageCreateInfo VertStageCreateInfo;
	VertStageCreateInfo.stage = vk::ShaderStageFlagBits::eVertex;
	VertStageCreateInfo.module = VertModule;
	VertStageCreateInfo.pName = "main";

	//Use SpirV_Reflect to build up our vertex input information
	SpvReflectShaderModule VertexShaderReflection;
	spvReflectCreateShaderModule(VertexSpirV.size(), VertexSpirV.data(), &VertexShaderReflection);

	uint32_t VertexInputCount = 0;
	spvReflectEnumerateInputVariables(&VertexShaderReflection, &VertexInputCount, nullptr);

	if (VertexInputCount > 0)
	{
		std::vector<SpvReflectInterfaceVariable*> VertexInputs(VertexInputCount);
		spvReflectEnumerateInputVariables(&VertexShaderReflection, &VertexInputCount, VertexInputs.data());

		//Sort by location so we can compute offsets below TODO: will need to sort by binding then location to handle multiple vertex buffers
		std::sort(std::begin(VertexInputs), std::end(VertexInputs),
		[](const SpvReflectInterfaceVariable* a, const SpvReflectInterfaceVariable* b) 
		{
			return a->location < b->location; 
		});


		uint32_t CurrentOffset = 0;

		//Individual elements of our vertices
		std::vector<vk::VertexInputAttributeDescription> InputAttributes;
		for (auto& Input : VertexInputs)
		{
			vk::VertexInputAttributeDescription Attribute;
			Attribute.binding = 0; //TODO: Allow multiple vertex buffer bindings
			Attribute.location = Input->location;
			Attribute.format   = (vk::Format)Input->format;
			Attribute.offset = CurrentOffset;

			InputAttributes.push_back(Attribute);

			std::cout << Input->name << std::endl;
			std::cout << Input->location << std::endl;
			std::cout << to_string(Attribute.format) << std::endl;
			std::cout << CurrentOffset << std::endl;
			std::cout << std::endl;

			CurrentOffset += spv_reflect::FormatSize((VkFormat) Attribute.format);		
		}

		//Represents one type of Vertex for an input vertex buffer
		vk::VertexInputBindingDescription VertexBinding;
		VertexBinding.binding = 0; //TODO: Allow multiple vertex buffer bindings
		VertexBinding.stride = CurrentOffset;
		VertexBinding.inputRate = vk::VertexInputRate::eVertex;

		std::vector<vk::VertexInputBindingDescription> VertexBindingArray;
		VertexBindingArray.push_back(VertexBinding);

		SetVertexInputBindings(VertexBindingArray, InputAttributes);
	}

	spvReflectDestroyShaderModule(&VertexShaderReflection);

	std::vector<char> FragmentSpirV = LoadShaderFromFile(FragmentShader);
	vk::ShaderModule FragModule = CreateShaderModule(FragmentSpirV);
	vk::PipelineShaderStageCreateInfo FragStageCreateInfo;
	FragStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
	FragStageCreateInfo.module = FragModule;
	FragStageCreateInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo ShaderStages[] = {VertStageCreateInfo, FragStageCreateInfo};

	//Actually hook up Shader Stages
	CreateInfo.stageCount = 2;
	CreateInfo.pStages = ShaderStages;

	//Fixed function create infos
	CreateInfo.pVertexInputState = &VertexInput;
	CreateInfo.pInputAssemblyState = &InputAssembly;
	CreateInfo.pTessellationState = &Tessellation;

	ViewportState.viewportCount = 1;
	ViewportState.pViewports = &Viewport;
	ViewportState.scissorCount = 1;
	ViewportState.pScissors = &Scissor;
	CreateInfo.pViewportState = &ViewportState;

	CreateInfo.pRasterizationState = &Rasterizer;
	CreateInfo.pMultisampleState = &Multisampling;
	CreateInfo.pDepthStencilState = &DepthStencil;
	CreateInfo.pColorBlendState = &ColorBlending;

	//Add Dynamic State if array is nonempty
	DynamicState.dynamicStateCount = (uint32_t) DynamicStates.size();
	DynamicState.pDynamicStates = DynamicStates.data();
	CreateInfo.pDynamicState = (DynamicStates.size() > 0) ? &DynamicState : nullptr;

	//Pipeline Layout
	PipelineLayout = VulkanContext::Get()->GetDevice().createPipelineLayoutUnique(PipelineLayoutCreateInfo);
	CreateInfo.layout = PipelineLayout.get();
	
	//TODO: Hook up Layout, Renderpass, and subpass vars in struct
	//Render pass hookup
	CreateInfo.renderPass = RenderPass.GetRenderPass();
	CreateInfo.subpass = 0;
	
	GraphicsPipeline = VulkanContext::Get()->GetDevice().createGraphicsPipelineUnique(vk::PipelineCache(), CreateInfo);

	//Done with shader modules
	VulkanContext::Get()->GetDevice().destroyShaderModule(VertModule);
	VulkanContext::Get()->GetDevice().destroyShaderModule(FragModule);
}

std::vector<char> VulkanGraphicsPipeline::LoadShaderFromFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) 
	{
		std::cout << "Failed to load shader: " << filename << std::endl;
		throw std::runtime_error("failed to open file: " + filename);
    }

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> code(fileSize);
	
	file.seekg(0);
	file.read(code.data(), fileSize);

	file.close();

	return code;
}

vk::ShaderModule VulkanGraphicsPipeline::CreateShaderModule(std::vector<char> spvCode)
{
	vk::ShaderModuleCreateInfo CreateInfo;
	CreateInfo.codeSize = spvCode.size();
	CreateInfo.pCode = reinterpret_cast<const uint32_t*>(spvCode.data());

	return VulkanContext::Get()->GetDevice().createShaderModule(CreateInfo);
}

void VulkanGraphicsPipeline::SetVertexInputBindings(std::vector<vk::VertexInputBindingDescription>& InputBindings, std::vector<vk::VertexInputAttributeDescription>& AttributeBindings)
{
	//Store this information so our pointers are guaranteed valid below
	VertexInputBindings = InputBindings;
	VertexAttributeBindings = AttributeBindings;

	VertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(VertexInputBindings.size());
	VertexInput.pVertexBindingDescriptions = VertexInputBindings.data();

	VertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexAttributeBindings.size());
	VertexInput.pVertexAttributeDescriptions = VertexAttributeBindings.data();
}