#include "VulkanContext.h"
#include <vulkan/vulkan.hpp>
#include <vector>
#include <set>
#include <iostream>
#include <GLFW\glfw3.h>

#define ENABLE_VK_VALIDATION 1
#define VERBOSE_VALIDATION 0

VulkanContext *VulkanContext::SingletonPtr = nullptr;

VulkanContext::VulkanContext()
{

}

VulkanContext::~VulkanContext()
{

}

void VulkanContext::Startup(GLFWwindow* window)
{
    std::cout << "--- BEGIN VULKAN RENDERER STARTUP ---" << std::endl;
    CreateInstance();
    SetupDebugCallback();
	CreateGLFWSurface(window);
    CreateDeviceAndQueues();
	CreateCommandPool();
}

void VulkanContext::Shutdown()
{
    std::cout << "--- BEGIN VULKAN RENDERER SHUTDOWN ---" << std::endl;

	Device.destroyCommandPool(CommandPool, nullptr);
    Device.destroy(nullptr);
    RemoveDebugCallback();
    Instance.destroy(nullptr);
}

void VulkanContext::CreateInstance()
{
    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "Scalpel";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Scalpel Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;

    //GLFW Specific Extensions
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> enabledExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#if ENABLE_VK_VALIDATION
    //Validation layer extension
    enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

    if (CheckExtensionSupport(enabledExtensions))
    {
        createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());;
        createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    }
    
#if ENABLE_VK_VALIDATION  
    std::vector<const char*> validationLayers;
    validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");

    const bool bLayersSupported = CheckValidationLayerSupport(validationLayers);
    if (bLayersSupported)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }   
#endif

    try
    {
        Instance = vk::createInstance(createInfo);
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception Thrown on vkCreateInstance: "<< e.what() << std::endl;
    }
}

bool VulkanContext::CheckExtensionSupport(const std::vector<const char*>& extensions)
{
    uint32_t extensionCount;
    vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
    vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    for (const char* extensionName : extensions) 
    {
        bool extensionFound = false;

        for (const auto& extensionProperties : availableExtensions) 
        {
            if (strcmp(extensionName, extensionProperties.extensionName) == 0) 
            {
                extensionFound = true;
                break;
            }
        }

        if (!extensionFound) 
        {
            std::cout << "Failed to find requested extension: "  << extensionName << std::endl;
            return false;
        }
    }

    return true;
}

bool VulkanContext::CheckValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
    uint32_t layerCount;
    vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<vk::LayerProperties> availableLayers(layerCount);
    vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) 
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) 
        {
            if (strcmp(layerName, layerProperties.layerName) == 0) 
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) 
        {
            std::cout << "Failed to find requested validation layer: "  << layerName << std::endl;
            return false;
        }
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData) 
{

    std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}

//Free Function to wrap function pointer 
VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) 
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr) 
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    } else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(instance, callback, pAllocator);
    }
}

void VulkanContext::SetupDebugCallback()
{
#if ENABLE_VK_VALIDATION
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | //Standard Validation Bits
        (VERBOSE_VALIDATION & (VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)); //Verbose Validation Bits
    createInfo.pfnCallback = debugCallback;

    if (CreateDebugReportCallbackEXT(Instance, &createInfo, nullptr, &callback) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to set up debug callback!");
    }

#endif
}

void VulkanContext::RemoveDebugCallback()
{
#if ENABLE_VK_VALIDATION
    DestroyDebugReportCallbackEXT(Instance, callback, nullptr);
#endif
}

void VulkanContext::CreateDeviceAndQueues()
{  
    //Actually fetch the physical devices
    std::vector<vk::PhysicalDevice> Devices = Instance.enumeratePhysicalDevices();
    
    //Find A device that supports graphics queue
    for (auto CurrPhysicalDevice : Devices)
    { 
        //Get Physical Device Properties
        vk::PhysicalDeviceProperties PhysicalDeviceProperties = CurrPhysicalDevice.getProperties();

        //Actually Get Queue Family Properties
		std::vector<vk::QueueFamilyProperties> QueueFamilyProperties = CurrPhysicalDevice.getQueueFamilyProperties();

        //TODO: Check device for needed features here before committing to it (64 bit fp ops, texture compression, etc.)
        for (int i = 0; i < QueueFamilyProperties.size(); ++i)
        {
			//Need queue that supports graphics and presnetation
            if (QueueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
            {
                GraphicsQueueIndex = i;
                std::cout << "Using Device with Graphics Queue Support: " << PhysicalDeviceProperties.deviceName <<  " | Graphics Index: " << i << std::endl;  
            }

			//Ensure our graphics queue also has presentation support 
			if (CurrPhysicalDevice.getSurfaceSupportKHR(i, Surface))
			{
				PresentQueueIndex = i;
				std::cout << "Using Device with Presentation Queue Support:"  << PhysicalDeviceProperties.deviceName <<  " | Presentation Index: "<< i << std::endl;
			}

			if (GraphicsQueueIndex != -1 && PresentQueueIndex != -1)
			{
				//Found the device that supports our needs
				PhysicalDevice = CurrPhysicalDevice;
				break;
			}
        }
    }

	std::vector<vk::DeviceQueueCreateInfo> QueueCreateInfos;
	std::set<int> UniqueQueueFamilies = {GraphicsQueueIndex, PresentQueueIndex};

	float QueuePriority = 1.0f;
	for (int QueueFamily : UniqueQueueFamilies) 
	{
		vk::DeviceQueueCreateInfo QueueCreateInfo;
		QueueCreateInfo.queueFamilyIndex = QueueFamily;
		QueueCreateInfo.queueCount = 1;
		QueueCreateInfo.pQueuePriorities = &QueuePriority;
		QueueCreateInfos.push_back(QueueCreateInfo);
	}

    vk::DeviceCreateInfo DeviceCreateInfo = {};
    DeviceCreateInfo.queueCreateInfoCount =static_cast<uint32_t>(QueueCreateInfos.size());
    DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();

    std::vector<const char*> DeviceLayers;

#ifdef ENABLE_VK_VALIDATION //TODO Validation
    DeviceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

    DeviceCreateInfo.ppEnabledLayerNames = DeviceLayers.data();
    DeviceCreateInfo.enabledLayerCount = static_cast<uint32_t> (DeviceLayers.size());

    std::vector<const char*> deviceExtensions =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    DeviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t> (deviceExtensions.size());
    Device = PhysicalDevice.createDevice(DeviceCreateInfo, nullptr);

    GraphicsQueue = Device.getQueue(GraphicsQueueIndex, 0);
	PresentQueue = Device.getQueue(PresentQueueIndex, 0);
}

void VulkanContext::CreateCommandPool()
{
	vk::CommandPoolCreateInfo CreateInfo;
	CreateInfo.queueFamilyIndex = GraphicsQueueIndex;

	CommandPool = Device.createCommandPool(CreateInfo);
}

void VulkanContext::CreateGLFWSurface(GLFWwindow* window)
{
    VkSurfaceKHR tmp;
    if (glfwCreateWindowSurface(VulkanContext::Get()->GetInstance(), window, nullptr, &tmp) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create window surface!");
    }

    std::cout << "GLFW Surface Successfully Created" << std::endl;
    Surface = tmp;
}