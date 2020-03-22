#define NOMINMAX

#include <string>
#include <iostream>
#include <algorithm>

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

#include "Renderer/GLSL/ShaderCompiler.hpp"

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
			indices.push_back((uint32_t)indices.size());
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
		CamToTarget = glm::mat3(glm::rotate(-.4f * MouseDeltaX, CamUp)) * CamToTarget;

		//Pitch
		CamToTarget = glm::mat3(glm::rotate(-.4f * MouseDeltaY, CamRight)) * CamToTarget;

		Target = CameraPosition + CamToTarget;
	}
}

int main(int, char**)
{
	auto VertSpv = CompileGLSL(ASSET_DIR + std::string("/shaders/shader.vert"));
	auto FragSpv = CompileGLSL(ASSET_DIR + std::string("/shaders/shader.frag"));

	// Setup window
	auto error_callback = [] (int error, const char* description)
	{
		std::cout << "Error: " << error << " desc: " << description << std::endl;	
	};

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {return 1; }

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	
	int InitialWidth = 1280;
	int InitialHeight = 720;

	GLFWwindow* window = glfwCreateWindow(InitialWidth, InitialHeight, "Scalpel", NULL, NULL);

	VulkanContext* Context = VulkanContext::Get();
	Context->Startup(window);
	
	//Scope block for implicit destruction of unique vulkan objects
	{
		//Testing adding an additional render target
		VulkanImage RenderTargetImage(InitialWidth, InitialHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
		VulkanRenderTarget TestRenderTarget;
		for (auto& UniqueImageView : Context->GetSwapchain().GetImageViews())
		{
			TestRenderTarget.ImageViews.push_back(&RenderTargetImage.GetImageView());
		}
		TestRenderTarget.Format = RenderTargetImage.GetFormat();
		TestRenderTarget.ClearValue = vk::ClearColorValue(std::array<float, 4>{.81f, 0.21f, 0.48f, 1.0f});

		VulkanRenderTarget ColorTarget;
		for (auto& UniqueImageView : Context->GetSwapchain().GetImageViews())
		{
			ColorTarget.ImageViews.push_back(&UniqueImageView.get());
		}
		ColorTarget.Format = Context->GetSwapchain().GetColorFormat();
		ColorTarget.ClearValue = vk::ClearColorValue(std::array<float, 4>{.41f, 0.61f, 0.88f, 1.0f});
		ColorTarget.InitialLayout = vk::ImageLayout::eUndefined;
		ColorTarget.UsageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		ColorTarget.FinalLayout = vk::ImageLayout::ePresentSrcKHR;

		VulkanRenderTarget DepthTarget;
		for (auto& UniqueImageView : Context->GetSwapchain().GetImageViews())
		{
			DepthTarget.ImageViews.push_back(&Context->GetSwapchain().GetDepthView());
		}
		DepthTarget.Format = Context->GetSwapchain().GetDepthFormat();
		DepthTarget.ClearValue = vk::ClearDepthStencilValue(1.0f, 0);
		DepthTarget.StoreOp = vk::AttachmentStoreOp::eDontCare;
		DepthTarget.UsageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
		DepthTarget.FinalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		//Note: currently the order in this array determines index in shader
		std::vector<VulkanRenderTarget*> ColorTargets = { &ColorTarget, &TestRenderTarget};

		VulkanRenderPass RenderPass;
		RenderPass.BuildRenderPass(ColorTargets, &DepthTarget, Context->GetSwapchain().GetExtent().width, Context->GetSwapchain().GetExtent().height, (uint32_t)Context->GetSwapchain().GetImageViews().size());

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
		TestVulkanRenderItem.AddBufferResource("MVP", UniformBuffer.GetDescriptorInfo());
		TestVulkanRenderItem.AddImageResource("texSampler", Image.GetDescriptorInfo());
		
		glm::vec3 CameraPosition(0.0f, 2.0f, 2.0f);
		glm::vec3 Target(0,0,0);
		const glm::vec3 UpVector(0,0,1);

		auto UpdateUniformData = [&] (VulkanUniform& Uniform, const float& deltaSeconds)
		{
			static float TotalTime = 0.0f;
			TotalTime += deltaSeconds;

			UniformBufferObject Ubo;
			Ubo.model = glm::rotate(glm::mat4(1.0f), TotalTime * glm::radians(27.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			Ubo.view = glm::lookAt(CameraPosition, Target, UpVector);
			Ubo.proj = glm::perspective(glm::radians(60.0f), Context->GetSwapchain().GetExtent().width / (float) Context->GetSwapchain().GetExtent().height, 0.001f, 10000.0f);
			Ubo.proj[1][1] *= -1;

			Uniform.UpdateUniformData(&Ubo, sizeof(UniformBufferObject));
		};

		//TODO: Pipeline derivation (faster creation, faster binding) (can derive parts of the create info)		
		VulkanGraphicsPipeline Pipeline;

		Pipeline.InputAssembly.topology = vk::PrimitiveTopology::eTriangleList;

		Pipeline.DepthStencil.depthTestEnable = VK_TRUE;
		Pipeline.DepthStencil.depthWriteEnable = VK_TRUE;

		Pipeline.BuildPipeline(RenderPass, VertSpv, FragSpv);
		/* ... End Pipeline Setup ... */

		std::vector<VulkanCommandBuffer> CommandBuffers;
		CommandBuffers.resize(RenderPass.GetFramebuffers().size());

		std::vector<std::pair<VulkanRenderItem*, VulkanGraphicsPipeline*>> VulkanRenderItems;
		for (int i = 0; i < 1000; ++i)
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

			auto HandleResize = [&]()
			{
				Context->GetDevice().waitIdle();

				Context->GetSwapchain().Build();

				//TODO: This is what needs to be done after a resize for a rendertarget
				{
					RenderTargetImage = VulkanImage(Context->GetSwapchain().GetExtent().width, Context->GetSwapchain().GetExtent().height, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);
					TestRenderTarget.ImageViews.clear();
					for (auto& UniqueImageView : Context->GetSwapchain().GetImageViews())
					{
						TestRenderTarget.ImageViews.push_back(&RenderTargetImage.GetImageView());
					}
				}
				
				RenderPass.BuildRenderPass(ColorTargets, &DepthTarget, Context->GetSwapchain().GetExtent().width, Context->GetSwapchain().GetExtent().height, (uint32_t)Context->GetSwapchain().GetImageViews().size());

				Pipeline.BuildPipeline(RenderPass, VertSpv, FragSpv);

				BuildPrimaryCommandBuffers();
			};

			//Handle Resize (can still try to acquire our image this frame)
			if (NewWidth != Width || NewHeight != Height)
			{
				Width = NewWidth;
				Height = NewHeight;
				HandleResize();
			}

			auto NextImage = Context->GetDevice().acquireNextImageKHR(Context->GetSwapchain().GetHandle(), std::numeric_limits<uint64_t>::max(), ImageAvailableSemaphore.get(), vk::Fence());
			
			if (NextImage.result == vk::Result::eErrorOutOfDateKHR || NextImage.result == vk::Result::eSuboptimalKHR)
			{
				//Acquiring Image failed: Need to rebuild our Out-Of-Date or SubOptimal swapchain
				HandleResize();
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

			vk::SwapchainKHR SwapChains[] = {Context->GetSwapchain().GetHandle()};
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
