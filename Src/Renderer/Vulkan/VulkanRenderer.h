#pragma once

#include <vulkan/vulkan.hpp>

//Vulkan Renderer Singleton Class
//Manages long-persisting vulkan data structures
//Instance, Devices, Queues, CommandPool
class VulkanRenderer
{
    VulkanRenderer();

public:
    ~VulkanRenderer();

    virtual void Startup();

    virtual void Shutdown();

public: //Helper Functions

    //Instance Creation (and validation layer setup)
    void CreateInstance();
    bool CheckExtensionSupport(const std::vector<const char*>& extensions);
    bool CheckValidationLayerSupport(const std::vector<const char*>& validationLayers);
    void SetupDebugCallback();
    void RemoveDebugCallback();

    //Physical and Logical device creation
    void CreateDeviceAndQueue();
	vk::Device GetDevice() {return Device;}
	vk::Queue GetQueue() {return GraphicsQueue;}

	//Creates a command pool from which to create command buffers
	void CreateCommandPool();
	vk::CommandPool GetCommandPool() {return CommandPool;}

    //Platform-Specific Surface Creation
    void CreateGLFWSurface(struct GLFWwindow* window);
    
protected:

    vk::Instance Instance;

    VkDebugReportCallbackEXT callback;

    vk::PhysicalDevice PhysicalDevice;
    vk::Device Device;

    vk::Queue GraphicsQueue;
	int GraphicsQueueIndex = -1;

    vk::SurfaceKHR Surface;

	vk::CommandPool CommandPool;

public:

    static VulkanRenderer *Get()
    {
        if (!SingletonPtr)
		{
          SingletonPtr = new VulkanRenderer;
		}

        return SingletonPtr;
    }

private:

	static VulkanRenderer* SingletonPtr;
};