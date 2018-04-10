#include <stdio.h>
#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <memory>
#include <chrono>

#include "Renderer/Vulkan/VulkanContext.h"
#include "Renderer/Vulkan/VulkanCommandBuffer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanGraphicsPipeline.h"
#include "Renderer/Vulkan/VulkanRenderPass.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanUniform.h"
#include "Renderer/Vulkan/VulkanImage.h"

#include <GLFW\glfw3.h>

#define VULKAN_HPP_NO_EXCEPTIONS

void HandleInput(GLFWwindow* window, const float& deltaSeconds, const float& MouseDeltaX, const float& MouseDeltaY, glm::vec3& CameraPosition, glm::vec3& Target)
{
	const float MoveSpeed = 3.0f * deltaSeconds;

	glm::vec3 CamForward = glm::normalize(Target - CameraPosition) * MoveSpeed;
	glm::vec3 CamRight   = glm::normalize(glm::cross(CamForward, glm::vec3(0,0,1))) * MoveSpeed;
	glm::vec3 CamUp      = glm::normalize(glm::cross(CamRight, CamForward)) * MoveSpeed;
	
	//TODO: Have Key Callback write to a std::map<int, bool> that maps keycodes to their pressed state
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		CameraPosition += CamForward;
		Target += CamForward;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		CameraPosition -= CamForward;
		Target -= CamForward;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		CameraPosition -= CamRight;
		Target -= CamRight;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		CameraPosition += CamRight;
		Target += CamRight;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		CameraPosition -= CamUp;
		Target -= CamUp;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		CameraPosition += CamUp;
		Target += CamUp;
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	//Camera Mouse Rotation
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		glm::vec3 CamToTarget = glm::normalize(Target - CameraPosition);

		//Yaw
		CamToTarget = glm::mat3(glm::rotate(-7.0f * MouseDeltaX, CamUp)) * CamToTarget;

		//Pitch
		CamToTarget = glm::mat3(glm::rotate(-7.0f * MouseDeltaY, CamRight)) * CamToTarget;

		Target = CameraPosition + CamToTarget;
	}
}

int main(int, char**)
{
	// Setup window
	auto error_callback = [] (int error, const char* description)
	{
		std::cout << "Error: " << error << " desc: " << description << std::endl;	
	};

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) 
	{
		return 1;
	}

	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	GLFWwindow* window = glfwCreateWindow(1080, 720, "Scalpel", NULL, NULL);

	VulkanContext* Context = VulkanContext::Get();
	Context->Startup(window);
	
	//Scope block for implicit destruction of unique vulkan objects
	{
		VulkanSwapchain Swapchain;
		Swapchain.BuildSwapchain();

		VulkanRenderPass RenderPass;
		RenderPass.BuildRenderPass(Swapchain);

		VulkanBuffer VertexBuffer((void*) vertices.data(), sizeof(vertices[0]) * vertices.size(), EBufferType::VertexBuffer);
		VulkanBuffer IndexBuffer((void*) indices.data(), sizeof(indices[0]) * indices.size(), EBufferType::IndexBuffer);

		std::string ImageName(ASSET_DIR + std::string("/textures/test.png"));
		VulkanImage Image(ImageName);
		vk::ImageView ImageView = Image.GetImageView();
		vk::Sampler ImageSampler = Image.GetSampler();

		struct UniformBufferObject {
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		VulkanUniform UniformBuffer(sizeof(UniformBufferObject));

		glm::vec3 CameraPosition(0.0f, 2.0f, 2.0f);
		glm::vec3 Target(0,0,0);
		const glm::vec3 UpVector(0,0,1);

		auto UpdateUniformData = [&] (VulkanUniform& Uniform, const float& deltaSeconds)
		{
			static float TotalTime = 0.0f;
			TotalTime += deltaSeconds;

			UniformBufferObject Ubo;
			Ubo.model = glm::rotate(glm::mat4(1.0f), TotalTime * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			Ubo.view = glm::lookAt(CameraPosition, Target, UpVector);
			Ubo.proj =  glm::perspective(glm::radians(60.0f), Swapchain.GetExtent().width / (float) Swapchain.GetExtent().height, 0.001f, 10000.0f);
			Ubo.proj[1][1] *= -1;

			Uniform.UpdateUniformData(&Ubo, sizeof(UniformBufferObject));
		};

		/* ... Pipeline Setup Here ... */
		//TODO: Better way to deal with all of this pipeline setup (perhaps a pipeline definition file that goes along with Shader SpirV)
		VulkanGraphicsPipeline Pipeline;

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
		Pipeline.Rasterizer.cullMode = vk::CullModeFlagBits::eNone;
		Pipeline.Rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
		Pipeline.Rasterizer.depthBiasEnable = VK_FALSE;

		Pipeline.Multisampling.sampleShadingEnable = VK_FALSE;
		Pipeline.Multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

		Pipeline.ColorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR 
													 | vk::ColorComponentFlagBits::eG 
													 | vk::ColorComponentFlagBits::eB
													 | vk::ColorComponentFlagBits::eA;
		Pipeline.ColorBlendAttachment.blendEnable = VK_FALSE;

		Pipeline.ColorBlending.logicOpEnable = VK_FALSE;
		Pipeline.ColorBlending.logicOp = vk::LogicOp::eCopy; // Optional when off
		Pipeline.ColorBlending.attachmentCount = 1;
		Pipeline.ColorBlending.pAttachments = &Pipeline.ColorBlendAttachment;
		Pipeline.ColorBlending.blendConstants[0] = 0.0f;
        Pipeline.ColorBlending.blendConstants[1] = 0.0f;
        Pipeline.ColorBlending.blendConstants[2] = 0.0f;
		Pipeline.ColorBlending.blendConstants[3] = 0.0f;

		Pipeline.DepthStencil.depthTestEnable = VK_TRUE;
		Pipeline.DepthStencil.depthWriteEnable = VK_TRUE;
		Pipeline.DepthStencil.depthCompareOp = vk::CompareOp::eLess;
		Pipeline.DepthStencil.minDepthBounds = 0.0f;
		Pipeline.DepthStencil.maxDepthBounds = 1.0f;
		Pipeline.DepthStencil.stencilTestEnable = VK_FALSE;

		//Pipeline.DynamicStates.push_back(vk::DynamicState::eViewport);
		
		Pipeline.BuildPipeline(RenderPass, ASSET_DIR + std::string("/shaders/vert.spv"), ASSET_DIR + std::string("/shaders/frag.spv"));
		/* ... End Pipeline Setup ... */
	
		/*TODO: std::pair<vk::UniqueDescriptorPool,std::vector<vk::UniqueDescriptorSet>> is a bit verbose, 
		   and "first" and "second" don't really do a good job of describing what they hold */
		auto DescriptorPoolAndSets = Pipeline.AllocateDescriptorSets(1);
		vk::UniqueDescriptorSet& DescriptorSet = DescriptorPoolAndSets.second[0]; //Just for ease of access later

		//Actually reference our image view and sampler
		std::array<vk::WriteDescriptorSet,2> DescriptorWrites;
		DescriptorWrites[0].dstSet = DescriptorSet.get();
		DescriptorWrites[0].dstBinding = 0;
		DescriptorWrites[0].dstArrayElement = 0;
		DescriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
		DescriptorWrites[0].descriptorCount = 1;
		DescriptorWrites[0].pBufferInfo = &UniformBuffer.GetDescriptorInfo();

		DescriptorWrites[1].dstSet = DescriptorSet.get();
		DescriptorWrites[1].dstBinding = 1;
		DescriptorWrites[1].dstArrayElement = 0;
		DescriptorWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		DescriptorWrites[1].descriptorCount = 1;
		DescriptorWrites[1].pImageInfo = &Image.GetDescriptorInfo();

		Context->GetDevice().updateDescriptorSets(DescriptorWrites, nullptr);

		//END DESCRIPTOR SET AND DESCRIPTOR SET LAYOUT SETUP

		std::vector<VulkanCommandBuffer> CommandBuffers;
		CommandBuffers.resize(RenderPass.GetFramebuffers().size());

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
				
				vk::ClearColorValue ClearColor(std::array<float, 4>{0.39f, 0.58f, 0.93f, 1.0f});
				vk::ClearDepthStencilValue ClearDepth(1.0f, 0);
				std::vector<vk::ClearValue> ClearValues = {ClearColor, ClearDepth};
				
				BeginInfo.clearValueCount = static_cast<uint32_t>(ClearValues.size());
				BeginInfo.pClearValues = ClearValues.data();
				CmdBuffer().beginRenderPass(BeginInfo, vk::SubpassContents::eInline);
				CmdBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, Pipeline.GetHandle());
				
				vk::Buffer VertexBuffers[] = {VertexBuffer.GetHandle()};
				vk::DeviceSize Offsets[] = {0};
				
				CmdBuffer().bindVertexBuffers(0, 1, VertexBuffers, Offsets);
				CmdBuffer().bindIndexBuffer(IndexBuffer.GetHandle(), 0, vk::IndexType::eUint16);
				CmdBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Pipeline.GetLayout(), 0, 1, &DescriptorSet.get(), 0, nullptr);
				CmdBuffer().drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				CmdBuffer().endRenderPass();
				CmdBuffer.End();
			}
		};

		BuildDrawingCommandBuffers();

		vk::UniqueSemaphore ImageAvailableSemaphore = Context->GetDevice().createSemaphoreUnique(vk::SemaphoreCreateInfo());
		vk::UniqueSemaphore RenderFinishedSemaphore = Context->GetDevice().createSemaphoreUnique(vk::SemaphoreCreateInfo());

		int Width, Height;
		glfwGetWindowSize(window, &Width, &Height);

		// Main loop
		double LastTime = 0.0;
		float deltaSeconds = 0.0;
		
		double LastMouseX, LastMouseY;
		glfwGetCursorPos(window, &LastMouseX, &LastMouseY);

		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();

			//Calculate deltaSeconds
			double CurrentTime = glfwGetTime();
			deltaSeconds = (float)(CurrentTime - LastTime);

			LastTime = CurrentTime;

			double MouseX, MouseY;
			glfwGetCursorPos(window, &MouseX, &MouseY);

			double MouseDeltaX = (MouseX - LastMouseX) * deltaSeconds;
			double MouseDeltaY = (MouseY - LastMouseY) * deltaSeconds;

			HandleInput(window, deltaSeconds, (float)MouseDeltaX, (float)MouseDeltaY, CameraPosition, Target);
			
			LastMouseX = MouseX;
			LastMouseY = MouseY;

			//Window Resizing Logic
			int NewWidth, NewHeight;
			glfwGetWindowSize(window, &NewWidth, &NewHeight);

			//If window was minimized, don't render
			if (NewWidth == 0 || NewHeight == 0)
			{
				continue;
			}

			UpdateUniformData(UniformBuffer, deltaSeconds);

			auto RebuildSwapchain = [&]()
			{
				Context->GetDevice().waitIdle();

				Swapchain.BuildSwapchain();
				
				RenderPass.BuildRenderPass(Swapchain);

				Pipeline.Viewport.width = (float) Swapchain.GetExtent().width;
				Pipeline.Viewport.height = (float) Swapchain.GetExtent().height;
				Pipeline.Scissor.extent = Swapchain.GetExtent();
				Pipeline.BuildPipeline(RenderPass, ASSET_DIR + std::string("/shaders/vert.spv"), ASSET_DIR + std::string("/shaders/frag.spv"));

				BuildDrawingCommandBuffers();

				Width = NewWidth;
				Height = NewHeight;
			};

			//Handle Resize (can still try to acquire our image this frame)
			if (NewWidth != Width || NewHeight != Height)
			{
				RebuildSwapchain();
			}

			auto NextImage = Context->GetDevice().acquireNextImageKHR(Swapchain.GetHandle(), std::numeric_limits<uint64_t>::max(), ImageAvailableSemaphore.get(), vk::Fence());
			if (NextImage.result == vk::Result::eErrorOutOfDateKHR || NextImage.result == vk::Result::eSuboptimalKHR)
			{
				//Acquiring Image failed: Need to rebuild our Out-Of-Date or SubOptimal swapchain
				RebuildSwapchain();
				continue;
			}

			uint32_t ImageIndex = NextImage.value;

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

			vk::Result Result = Context->GetPresentQueue().presentKHR(PresentInfo);
		
			Context->GetPresentQueue().waitIdle();
		}
		
		Context->GetDevice().waitIdle();
	}
	
	Context->Shutdown();

	// Cleanup
	glfwTerminate();

	return 0;
}
