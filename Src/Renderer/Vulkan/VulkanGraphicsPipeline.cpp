#include "VulkanGraphicsPipeline.h"

VulkanGraphicsPipeline::VulkanGraphicsPipeline()
{
	//Build up our create info struct, which BuildPipeline will use

	//TODO: actually hook up shader stages
	//TODO: Add additional shader stages

	//Shader Stage Create infos
	CreateInfo.stageCount = 2;
	//CreateInfo.pStages = 

	//Fixed function create infos
	CreateInfo.pVertexInputState = &VertexInput;
	CreateInfo.pInputAssemblyState = &InputAssembly;
	CreateInfo.pTessellationState = &Tessellation;
	CreateInfo.pViewportState = &ViewportState;
	CreateInfo.pRasterizationState = &Rasterizer;
	CreateInfo.pMultisampleState = &Multisampling;
	CreateInfo.pDepthStencilState = &DepthStencil;
	CreateInfo.pColorBlendState = &ColorBlending;
	CreateInfo.pDynamicState = &DynamicState;
	
	//TODO: Hook up Layout, Renderpass, and subpass vars in struct
	//Render pass hookup
	//CreateInfo.layout = 
	//CreateInfo.renderPass = 
	//CreateInfo.subpass = 0;
}

void VulkanGraphicsPipeline::BuildPipeline()
{	


}