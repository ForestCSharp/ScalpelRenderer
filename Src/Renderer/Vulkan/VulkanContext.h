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

    virtual void Startup();

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
    void CreateDeviceAndQueue();
	const vk::PhysicalDevice GetPhysicalDevice() {return PhysicalDevice;}
	const vk::Device GetDevice() {return Device;}
	const vk::Queue GetQueue() {return GraphicsQueue;}
	const int GetGraphicsQueueIndex() {return GraphicsQueueIndex;}

	//Creates a command pool from which to create command buffers
	void CreateCommandPool();
	vk::CommandPool GetCommandPool() {return CommandPool;}
    
protected:

    vk::Instance Instance;

    VkDebugReportCallbackEXT callback;

    vk::PhysicalDevice PhysicalDevice;
    vk::Device Device;

    vk::Queue GraphicsQueue;
	int GraphicsQueueIndex = -1;

	vk::CommandPool CommandPool;

public:

    static VulkanContext *Get()
    {
        if (!SingletonPtr)
		{
          SingletonPtr = new VulkanContext;
		}

        return SingletonPtr;
    }

private:

	static VulkanContext* SingletonPtr;
};