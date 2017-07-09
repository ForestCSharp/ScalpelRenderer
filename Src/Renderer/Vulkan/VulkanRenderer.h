#pragma once

#include <vulkan/vulkan.hpp>

class VulkanRenderer
{
public:

    VulkanRenderer();
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

    //Surface Creation
    void CreateSurface();
    
protected:

    vk::Instance Instance;
    VkDebugReportCallbackEXT callback;

    vk::PhysicalDevice PhysicalDevice;
    vk::Device Device;
    vk::Queue GraphicsQueue;

    vk::SurfaceKHR Surface;

};