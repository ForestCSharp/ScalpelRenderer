#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Renderer/Vulkan/VulkanContext.h"
#include "Renderer/Vulkan/VulkanCommandBuffer.h"
#include "Renderer/Vulkan/VulkanSwapchain.h"
#include "Renderer/Vulkan/VulkanGraphicsPipeline.h"
#include "Renderer/Vulkan/VulkanRenderPass.h"
#include "Renderer/Vulkan/VulkanBuffer.h"
#include "Renderer/Vulkan/VulkanUniform.h"
#include "Renderer/Vulkan/VulkanImage.h"
#include "Renderer/Vulkan/VulkanRenderItem.hpp"
#include <GLFW\glfw3.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "../Libs/tinyobj/tiny_obj_loader.h"

#define VULKAN_HPP_NO_EXCEPTIONS

VulkanRenderItem LoadModel(std::string& FilePath)
{
	tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, FilePath.c_str())) {
        throw std::runtime_error(err);
    }

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const auto& shape : shapes) 
	{
		for (const auto& index : shape.mesh.indices) 
		{
			Vertex vertex = {};

			vertex.pos = {
			attrib.vertices[3 * index.vertex_index + 0],
			attrib.vertices[3 * index.vertex_index + 1],
			attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
    			1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = {1.0f, 1.0f, 1.0f};

			vertices.push_back(vertex);
			indices.push_back(indices.size());
		}
	}

	VulkanRenderItem NewRenderItem((void*) vertices.data(), sizeof(vertices[0]) * vertices.size(),
								  (void*) indices.data(), sizeof(indices[0]) * indices.size(), static_cast<uint32_t>(indices.size()));

	return NewRenderItem;
}

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

	if (!glfwInit()) {return 1; }

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1080, 720, "Scalpel", NULL, NULL);

	VulkanContext* Context = VulkanContext::Get();
	Context->Startup(window);
	
	//Scope block for implicit destruction of unique vulkan objects
	{
		//TODO: For a game's purposes, this can probably be handled in the Context Startup
		VulkanSwapchain Swapchain;
		Swapchain.BuildSwapchain();

		//Swapchain Render Target Setup (this will be handled by Render Graph)
		/* TODO: implicitly determine LoadOp,StoreOp, layouts from usage as they move through the renderpasses 
				 Should only need to define format
				 see: https://www.ea.com/frostbite/news/framegraph-extensible-rendering-architecture-in-frostbite */
		VulkanRenderTarget ColorTarget;
		for (auto& UniqueImageView : Swapchain.GetImageViews())
		{
			ColorTarget.ImageViews.push_back(&UniqueImageView.get());
		}
		ColorTarget.Format = Swapchain.GetColorFormat();
		ColorTarget.LoadOp = vk::AttachmentLoadOp::eClear;
		ColorTarget.StoreOp = vk::AttachmentStoreOp::eStore;
		ColorTarget.InitialLayout = vk::ImageLayout::eUndefined;
		ColorTarget.UsageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		ColorTarget.FinalLayout = vk::ImageLayout::ePresentSrcKHR;

		VulkanRenderTarget DepthTarget;
		for (auto& UniqueImageView : Swapchain.GetImageViews())
		{
			DepthTarget.ImageViews.push_back(&Swapchain.GetDepthView());
		}
		DepthTarget.Format = Swapchain.GetDepthFormat();
		DepthTarget.LoadOp = vk::AttachmentLoadOp::eClear;
		DepthTarget.StoreOp = vk::AttachmentStoreOp::eDontCare;
		DepthTarget.InitialLayout = vk::ImageLayout::eUndefined;
		DepthTarget.UsageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		DepthTarget.FinalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		DepthTarget.bDepthTarget = true;

		std::vector<VulkanRenderTarget*> RenderTargets = {&ColorTarget, &DepthTarget};

		VulkanRenderPass RenderPass;
		RenderPass.BuildRenderPass(RenderTargets, Swapchain.GetExtent().width, Swapchain.GetExtent().height, Swapchain.GetImageViews().size());

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

		std::string ModelPath(ASSET_DIR + std::string("/models/Torus.obj"));
		VulkanRenderItem TestVulkanRenderItem = LoadModel(ModelPath);

		//Reference some resources in our render item
		TestVulkanRenderItem.AddBufferResource("ubo", UniformBuffer.GetDescriptorInfo());
		TestVulkanRenderItem.AddImageResource("texSampler", Image.GetDescriptorInfo());
		/*TODO: FIXME: Currently it will result in a crash if the RenderItem doesn't have a resource 
				for every descriptor in a descriptor set for a given pipeline */
		
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
			Ubo.proj = glm::perspective(glm::radians(60.0f), Swapchain.GetExtent().width / (float) Swapchain.GetExtent().height, 0.001f, 10000.0f);
			Ubo.proj[1][1] *= -1;

			Uniform.UpdateUniformData(&Ubo, sizeof(UniformBufferObject));
		};

		/* ... Pipeline Setup Here ... */
		//TODO: Better way to deal with all of this pipeline setup (perhaps a pipeline definition file that goes along with Shader SpirV)
		//TODO: Pipeline derivation (faster creation, faster binding) (can derive parts of the create info)
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

		std::vector<VulkanCommandBuffer> CommandBuffers;
		CommandBuffers.resize(RenderPass.GetFramebuffers().size());

		std::vector<std::pair<VulkanRenderItem*, VulkanGraphicsPipeline*>> VulkanRenderItems;
		for (int i = 0; i < 2; ++i)
		{
			VulkanRenderItems.push_back(std::pair<VulkanRenderItem*, VulkanGraphicsPipeline*>(&TestVulkanRenderItem, &Pipeline));
		}

		//Wrapped in lambda for window resize below
		auto BuildPrimaryCommandBuffers = [&]()
		{
			RenderPass.BuildCommandBuffer(VulkanRenderItems);

			for (size_t i = 0; i < CommandBuffers.size(); ++i)
			{
				auto& CommandBuffer = CommandBuffers[i];
				CommandBuffer.Begin();

				//TODO: Iterate over all renderpasses (sorted based on Frame Graph and call function to handle them (see below))
				//TODO: The above will also need to handle barriers between certain renderpasses when necessary

				RenderPass.RecordCommands(CommandBuffer, i);

				CommandBuffer.End();
			}
		};

		BuildPrimaryCommandBuffers();

		vk::UniqueSemaphore ImageAvailableSemaphore = Context->GetDevice().createSemaphoreUnique(vk::SemaphoreCreateInfo());
		vk::UniqueSemaphore RenderFinishedSemaphore = Context->GetDevice().createSemaphoreUnique(vk::SemaphoreCreateInfo());

		int Width, Height;
		glfwGetWindowSize(window, &Width, &Height);

		double LastTime = 0.0;
		float deltaSeconds = 0.0;
		
		double LastMouseX, LastMouseY;
		glfwGetCursorPos(window, &LastMouseX, &LastMouseY);

		// Main loop
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
				
				RenderPass.BuildRenderPass(RenderTargets, Swapchain.GetExtent().width, Swapchain.GetExtent().height, Swapchain.GetImageViews().size());

				Pipeline.Viewport.width = (float) Swapchain.GetExtent().width;
				Pipeline.Viewport.height = (float) Swapchain.GetExtent().height;
				Pipeline.Scissor.extent = Swapchain.GetExtent();
				Pipeline.BuildPipeline(RenderPass, ASSET_DIR + std::string("/shaders/vert.spv"), ASSET_DIR + std::string("/shaders/frag.spv"));

				BuildPrimaryCommandBuffers();
			};

			//Handle Resize (can still try to acquire our image this frame)
			if (NewWidth != Width || NewHeight != Height)
			{
				Width = NewWidth;
				Height = NewHeight;
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
			SubmitInfo.pCommandBuffers = &CommandBuffers[ImageIndex].GetHandle();
			
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
	
	// Cleanup
	Context->Shutdown();
	glfwTerminate();

	return 0;
}
