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
#include "Renderer/Vulkan/VulkanBuffer.h"

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
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	GLFWwindow* window = glfwCreateWindow(1080, 720, "Scalpel", NULL, NULL);
	glfwMakeContextCurrent(window);

	//TODO: VULKAN RENDERER TESTING
	VulkanContext* Context = VulkanContext::Get();
	Context->Startup(window);
	
	//Scope block for implicit destruction of unique vulkan objects
	{
		VulkanSwapchain Swapchain;
		Swapchain.BuildSwapchain();

		VulkanRenderPass RenderPass;
		RenderPass.BuildRenderPass(Swapchain);

		VulkanBuffer VertexBuffer;

		VulkanGraphicsPipeline Pipeline;

		/* ... Pipeline Setup Here ... */
		Pipeline.SetVertexInputBindings(Vertex::GetBindingDescriptions(), Vertex::GetAttributeDescriptions());
		
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

		Pipeline.ColorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR 
													 | vk::ColorComponentFlagBits::eG 
													 | vk::ColorComponentFlagBits::eB
													 | vk::ColorComponentFlagBits::eA;
		Pipeline.ColorBlendAttachment.blendEnable = VK_FALSE;

		Pipeline.ColorBlending.logicOpEnable = VK_FALSE;
		Pipeline.ColorBlending.logicOp = vk::LogicOp::eCopy; // Optional
		Pipeline.ColorBlending.attachmentCount = 1;
		Pipeline.ColorBlending.pAttachments = &Pipeline.ColorBlendAttachment;
		Pipeline.ColorBlending.blendConstants[0] = 0.0f;
        Pipeline.ColorBlending.blendConstants[1] = 0.0f;
        Pipeline.ColorBlending.blendConstants[2] = 0.0f;
        Pipeline.ColorBlending.blendConstants[3] = 0.0f;

		Pipeline.PipelineLayoutCreateInfo.setLayoutCount = 0;
		Pipeline.PipelineLayoutCreateInfo.pushConstantRangeCount = 0;

		Pipeline.DepthStencil.depthTestEnable = VK_TRUE;
		Pipeline.DepthStencil.depthWriteEnable = VK_TRUE;
		Pipeline.DepthStencil.depthCompareOp = vk::CompareOp::eLess;
		Pipeline.DepthStencil.minDepthBounds = 0.0f;
		Pipeline.DepthStencil.maxDepthBounds = 1.0f;
		Pipeline.DepthStencil.stencilTestEnable = VK_FALSE;

		//Pipeline.DynamicStates.push_back(vk::DynamicState::eViewport);

		/* ... End Pipeline Setup ... */
		Pipeline.BuildPipeline(RenderPass);

		std::vector<VulkanCommandBuffer> CommandBuffers;
		CommandBuffers.resize(RenderPass.GetFramebuffers().size());

		//TODO: Eventually these sort of command buffers will be tied to classes of things we need to draw
		// (i.e. static meshes, skinned meshes, etc.)
		//Wrapped in lambda for window resize below
		auto BuildDrawingCommandBuffers = [&]()
		{
			for (size_t i = 0; i < CommandBuffers.size(); ++i)
			{
				auto& CmdBuffer = CommandBuffers[i];
				CmdBuffer.Begin();
				/* ... Rendering Commands Here ... */
				vk::RenderPassBeginInfo BeginInfo;
				BeginInfo.renderPass = RenderPass.GetRenderPass();
				BeginInfo.framebuffer = RenderPass.GetFramebuffers()[i].get();
				BeginInfo.renderArea.offset = {0,0};
				BeginInfo.renderArea.extent = Swapchain.GetExtent();
				
				vk::ClearColorValue ClearColor(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});
				vk::ClearDepthStencilValue ClearDepth(1.0f, 0);
				std::vector<vk::ClearValue> ClearValues = {ClearColor, ClearDepth};
				
				BeginInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
				BeginInfo.pClearValues = ClearValues.data();
				CmdBuffer().beginRenderPass(BeginInfo, vk::SubpassContents::eInline);
				CmdBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, Pipeline.GetHandle());
				
				//Vertex Buffer Binding
				vk::Buffer VertexBuffers[] = {VertexBuffer.GetHandle()};
				vk::DeviceSize Offsets[] = {0};
				CmdBuffer().bindVertexBuffers(0, 1, VertexBuffers, Offsets);
				CmdBuffer().draw(static_cast<uint32_t>(vertices.size()),1,0,0);
				//End Vertex Buffer Binding

				CmdBuffer().endRenderPass();
				CmdBuffer.End();
			}
		};
		BuildDrawingCommandBuffers();

		vk::UniqueSemaphore ImageAvailableSemaphore = Context->GetDevice().createSemaphoreUnique(vk::SemaphoreCreateInfo());
		vk::UniqueSemaphore RenderFinishedSemaphore = Context->GetDevice().createSemaphoreUnique(vk::SemaphoreCreateInfo());

		int Width, Height;
		glfwGetWindowSize(window, &Width, &Height);
		int NewWidth, NewHeight;

		// Main loop
		while (!glfwWindowShouldClose(window))
		{
			//Window Resizing Logic
			glfwGetWindowSize(window, &NewWidth, &NewHeight);
			if (NewWidth != Width || NewHeight != Height)
			{
				Context->GetDevice().waitIdle();

				Swapchain.BuildSwapchain();
				
				RenderPass.BuildRenderPass(Swapchain);

				Pipeline.Viewport.width = (float) Swapchain.GetExtent().width;
				Pipeline.Viewport.height = (float) Swapchain.GetExtent().height;
				Pipeline.Scissor.extent = Swapchain.GetExtent();
				Pipeline.BuildPipeline(RenderPass);

				//Recall this lambda
				BuildDrawingCommandBuffers();

				Width = NewWidth;
				Height = NewHeight;
			}

			glfwPollEvents();

			uint32_t ImageIndex = Context->GetDevice().acquireNextImageKHR(Swapchain.GetHandle(), std::numeric_limits<uint64_t>::max(), ImageAvailableSemaphore.get(), vk::Fence()).value;

			vk::SubmitInfo SubmitInfo;
			vk::Semaphore WaitSemaphores[] = {ImageAvailableSemaphore.get()};
			const vk::PipelineStageFlags WaitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
			
			SubmitInfo.waitSemaphoreCount = 1;
			SubmitInfo.pWaitSemaphores = WaitSemaphores;
			SubmitInfo.pWaitDstStageMask = WaitStages;

			SubmitInfo.commandBufferCount = 1;
			SubmitInfo.pCommandBuffers = &CommandBuffers[ImageIndex].Get();
			
			vk::Semaphore SignalSemaphores[] = {RenderFinishedSemaphore.get()};
			SubmitInfo.signalSemaphoreCount = 1;
			SubmitInfo.pSignalSemaphores = SignalSemaphores;

			Context->GetGraphicsQueue().submit(1, &SubmitInfo, vk::Fence());

			vk::PresentInfoKHR PresentInfo;
			PresentInfo.waitSemaphoreCount = 1;
			PresentInfo.pWaitSemaphores = SignalSemaphores;

			vk::SwapchainKHR SwapChains[] = {Swapchain.GetHandle()};
			PresentInfo.swapchainCount = 1;
			PresentInfo.pSwapchains = SwapChains;

			PresentInfo.pImageIndices = &ImageIndex;

			Context->GetPresentQueue().presentKHR(PresentInfo);
		
			Context->GetPresentQueue().waitIdle();
		}
		
		Context->GetDevice().waitIdle();
	}
	
	Context->Shutdown();

	// Cleanup
	glfwTerminate();

	return 0;
}
