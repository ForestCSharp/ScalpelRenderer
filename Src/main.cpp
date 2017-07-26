#include <stdio.h>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "Renderer/Vulkan/VulkanContext.h"
#include "Renderer/Vulkan/VulkanCommandBuffer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanGraphicsPipeline.h"
#include "Renderer/Vulkan/VulkanRenderPass.h"

#include <GLFW\glfw3.h>

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

int main(int, char**)
{
	// Setup window
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		return 1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(1080, 720, "Scalpel", NULL, NULL);
	glfwMakeContextCurrent(window);

	//TODO: VULKAN RENDERER TESTING
	VulkanContext* v = VulkanContext::Get();
	v->Startup(window);
	
	//Scope block for implicit destruction of unique vulkan objects
	{
		VulkanSwapchain Swapchain;

		VulkanRenderPass RenderPass;
		/* ... Render Pass Setup Here */
		RenderPass.BuildRenderPass(Swapchain);

		VulkanGraphicsPipeline Pipeline;
		/* ... Pipeline Setup Here ... */
		
		Pipeline.InputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
		Pipeline.InputAssembly.primitiveRestartEnable = VK_FALSE;
		
		Pipeline.Viewport.x = 0.f;
		Pipeline.Viewport.y = 0.f;
		Pipeline.Viewport.width = (float) Swapchain.GetExtent().width;
		Pipeline.Viewport.height = (float) Swapchain.GetExtent().height;
		Pipeline.Viewport.minDepth = 0.f;
		Pipeline.Viewport.maxDepth = 1.f;
		Pipeline.Scissor.offset = {0,0};
		Pipeline.Scissor.extent = Swapchain.GetExtent();

		Pipeline.Rasterizer.depthClampEnable = VK_FALSE;
		Pipeline.Rasterizer.rasterizerDiscardEnable = VK_FALSE;
		Pipeline.Rasterizer.polygonMode = vk::PolygonMode::eFill;
		Pipeline.Rasterizer.lineWidth = 1.0f;
		Pipeline.Rasterizer.cullMode = vk::CullModeFlagBits::eBack;
		Pipeline.Rasterizer.frontFace = vk::FrontFace::eClockwise;
		Pipeline.Rasterizer.depthBiasEnable = VK_FALSE;

		Pipeline.Multisampling.sampleShadingEnable = VK_FALSE;
		Pipeline.Multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
		Pipeline.Multisampling.minSampleShading = 1.0f;

		Pipeline.ColorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR 
													 | vk::ColorComponentFlagBits::eG 
													 | vk::ColorComponentFlagBits::eB
													 | vk::ColorComponentFlagBits::eA;
		Pipeline.ColorBlendAttachment.blendEnable = VK_FALSE;
		Pipeline.ColorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
		Pipeline.ColorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
		Pipeline.ColorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		Pipeline.ColorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		Pipeline.ColorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		Pipeline.ColorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

		Pipeline.ColorBlending.logicOpEnable = VK_FALSE;
		Pipeline.ColorBlending.logicOp = vk::LogicOp::eCopy; // Optional
		Pipeline.ColorBlending.attachmentCount = 1;
		Pipeline.ColorBlending.pAttachments = &Pipeline.ColorBlendAttachment;

		Pipeline.DynamicStates.push_back(vk::DynamicState::eViewport);
		Pipeline.DynamicStates.push_back(vk::DynamicState::eLineWidth);

		/* ... End Pipeline Setup ... */
		Pipeline.BuildPipeline(RenderPass);

		VulkanCommandBuffer CmdBuffer;
		CmdBuffer.Begin();
		/* ... Rendering Commands Here ... */
		CmdBuffer.End();


		glClearColor(0, 0, 100, 1);

		// Main loop
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			//TODO: remove OpenGL Stuff
			int display_w, display_h;
			glfwGetFramebufferSize(window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glfwSwapBuffers(window);
		}
	}

	VulkanContext::Get()->Shutdown();

	// Cleanup
	glfwTerminate();

	return 0;
}
