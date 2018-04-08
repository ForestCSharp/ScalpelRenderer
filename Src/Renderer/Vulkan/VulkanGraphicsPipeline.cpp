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
	SPV_REFLECT_ASSERT(spvReflectCreateShaderModule(VertexSpirV.size(), VertexSpirV.data(), &VertexShaderReflection));

	uint32_t VertexInputCount = 0;
	SPV_REFLECT_ASSERT(spvReflectEnumerateInputVariables(&VertexShaderReflection, &VertexInputCount, nullptr));

	if (VertexInputCount > 0)
	{
		std::vector<SpvReflectInterfaceVariable*> VertexInputs(VertexInputCount);
		SPV_REFLECT_ASSERT(spvReflectEnumerateInputVariables(&VertexShaderReflection, &VertexInputCount, VertexInputs.data()));

		//TODO: will need to sort by binding THEN location to handle multiple vertex buffers
		std::sort(std::begin(VertexInputs), std::end(VertexInputs),
		[](const SpvReflectInterfaceVariable* a, const SpvReflectInterfaceVariable* b) 
		{
			return a->location < b->location; 
		});


		uint32_t CurrentOffset = 0;

		std::cout << "BEGIN VERTEX INPUT REFLECTION" << std::endl << std::endl;

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

	std::vector<char> FragmentSpirV = LoadShaderFromFile(FragmentShader);
	vk::ShaderModule FragModule = CreateShaderModule(FragmentSpirV);
	vk::PipelineShaderStageCreateInfo FragStageCreateInfo;
	FragStageCreateInfo.stage = vk::ShaderStageFlagBits::eFragment;
	FragStageCreateInfo.module = FragModule;
	FragStageCreateInfo.pName = "main";

	SpvReflectShaderModule FragmentShaderReflection;
	SPV_REFLECT_ASSERT(spvReflectCreateShaderModule(FragmentSpirV.size(), FragmentSpirV.data(), &FragmentShaderReflection));

	vk::PipelineShaderStageCreateInfo ShaderStages[] = {VertStageCreateInfo, FragStageCreateInfo};

	//Actually hook up Shader Stages
	CreateInfo.stageCount = 2;
	CreateInfo.pStages = ShaderStages;

	// TODO: For PipelineLayout, get necessary information from shaders [See Steps below]
	// [1] [DONE] Enumerate bindings for vertex shader
	// [2] [DONE] Enumerate bindings for fragment shader
	// [3] [DONE] Sort bindings by binding number to better find shared bindings
	// [4] iterate over sorted bindings and set up each (shared bindings having both eVertex and eFragment stage bits )
	// [For 4, if both stages use it, you would hit the binding twice, just need to |= the second flag bit]

	std::vector<vk::DescriptorSetLayoutBinding> DescriptorBindings;
	int LastEncounteredDescriptorBinding = -1;
	
	//TODO: Make this block generic and use it for both vert and frag sections
	//Inputs: Reflection Module, Shader Stage
	auto AddDescriptorBindingsFromShaderStage = [&] (const SpvReflectShaderModule* ShaderReflectionModule, const vk::ShaderStageFlagBits ShaderStage)
	{
		uint32_t DescriptorBindingCount = 0;
		SPV_REFLECT_ASSERT(spvReflectEnumerateDescriptorBindings(ShaderReflectionModule, &DescriptorBindingCount, nullptr));

		std::vector<SpvReflectDescriptorBinding*> ReflectionDescriptorBindings(DescriptorBindingCount);
		
		if (DescriptorBindingCount > 0)
		{
			SPV_REFLECT_ASSERT(spvReflectEnumerateDescriptorBindings(ShaderReflectionModule, &DescriptorBindingCount, ReflectionDescriptorBindings.data()));

			std::sort(std::begin(ReflectionDescriptorBindings), std::end(ReflectionDescriptorBindings),
			[](const SpvReflectDescriptorBinding* a, const SpvReflectDescriptorBinding* b) 
			{
				return a->binding < b->binding; 
			});

			for (auto& ReflectionDescriptorBinding : ReflectionDescriptorBindings)
			{
					//First see if a descriptor with this binding already exists in the array we're adding to
					if ((int)ReflectionDescriptorBinding->binding <= LastEncounteredDescriptorBinding)
					{
						for (auto& DescriptorBinding : DescriptorBindings)
						{
							//TODO: find correct existing binding and |= the shader stage of this reflection binding to it
						}
					}
					else //New Binding needs to be added
					{
						vk::DescriptorSetLayoutBinding DescriptorBinding = {0};
						DescriptorBinding.binding = ReflectionDescriptorBinding->binding;
						DescriptorBinding.descriptorType = (vk::DescriptorType)ReflectionDescriptorBinding->descriptor_type;
						DescriptorBinding.descriptorCount = 1; //TODO:
						DescriptorBinding.stageFlags = ShaderStage;

						DescriptorBindings.push_back(DescriptorBinding);
						LastEncounteredDescriptorBinding = DescriptorBinding.binding;
					}

					std::cout << ReflectionDescriptorBinding->binding << std::endl;
					std::cout << ReflectionDescriptorBinding->input_attachment_index << std::endl;
					std::cout << ReflectionDescriptorBinding->set << std::endl;
					std::cout << to_string((vk::DescriptorType)ReflectionDescriptorBinding->descriptor_type) << std::endl;
					std::cout << std::endl;
			}
		}
	};

	AddDescriptorBindingsFromShaderStage(&VertexShaderReflection,   vk::ShaderStageFlagBits::eVertex);
	AddDescriptorBindingsFromShaderStage(&FragmentShaderReflection, vk::ShaderStageFlagBits::eFragment);

	//Fixed function create infos (these member structs can be set before running "Build Pipeline")
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

	spvReflectDestroyShaderModule(&VertexShaderReflection);
	spvReflectDestroyShaderModule(&FragmentShaderReflection);
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