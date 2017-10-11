#pragma once

#include <vulkan/vulkan.hpp>

//Vulkan Renderer Singleton Class
//Manages long-persisting vulkan data structures
//Instance, Devices, Queues, CommandPool
class VulkanContext
{
    VulkanContext();

public:
    ~VulkanContext();

    virtual void Startup(struct GLFWwindow* window);

    virtual void Shutdown();

public: //Helper Functions

    //Instance Creation (and validation layer setup)
    void CreateInstance();
	vk::Instance GetInstance() {return Instance;}

    bool CheckExtensionSupport(const std::vector<const char*>& extensions);
    bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
    void SetupDebugCallback();
    void RemoveDebugCallback();

    //Physical and Logical device creation
    void CreateDeviceAndQueues();
	const vk::PhysicalDevice GetPhysicalDevice() {return PhysicalDevice;}
	const vk::Device GetDevice() {return Device;}
	const vk::Queue GetGraphicsQueue() {return GraphicsQueue;}
	const vk::Queue GetPresentQueue()  {return PresentQueue; }
	const int GetGraphicsQueueIndex() {return GraphicsQueueIndex;}
	const int GetPresentQueueIndex()  {return PresentQueueIndex; }

	//Creates a command pool from which to create command buffers
	void CreateCommandPool();
	vk::CommandPool GetCommandPool() {return CommandPool;}

	//Platform-Specific Surface Creation
    void CreateGLFWSurface(struct GLFWwindow* window);
	vk::SurfaceKHR GetSurface() {return Surface;}
    
protected:

    vk::Instance Instance;

    VkDebugReportCallbackEXT callback;

    vk::PhysicalDevice PhysicalDevice;
    vk::Device Device;

    vk::Queue GraphicsQueue;
	int GraphicsQueueIndex = -1;

	vk::Queue PresentQueue;
	int PresentQueueIndex = -1;

	vk::CommandPool CommandPool;

	vk::SurfaceKHR Surface;

public:

    static VulkanContext *Get()
    {
        if (!SingletonPtr)
		{
          SingletonPtr = new VulkanContext;
		}

        return SingletonPtr;
	}

	//TODO: This function doesn't really have a good place to live... 
	//		potentially abstract out Vulkan Memory Allocation?
	static uint32_t FindMemoryType(uint32_t TypeFilter, vk::MemoryPropertyFlags Properties);

	//Attempts to find a supported image format from candidates with tiling and features
	vk::Format FindSupportedFormat(const std::vector<vk::Format>& Candidates, vk::ImageTiling Tiling, vk::FormatFeatureFlags Features);

private:

	static VulkanContext* SingletonPtr;
};