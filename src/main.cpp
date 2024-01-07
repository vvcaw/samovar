#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <spdlog/spdlog.h>
#include <cstring>
#include <set>

const std::vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Extension functions to handle creation and destruction of debug listeners for validation layers. We have to fetch them here, as they are extensions and not necessarily included with vulkan per default.
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class SamovarApplication {
public:
    bool run() {
        const auto result = initWindow() && initVulkan() && mainLoop();
        cleanup();
        return result;
    }

private:
    GLFWwindow *pWindow;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    bool initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        pWindow = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
        return pWindow != nullptr;
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char *layerName: validationLayers) {
            bool layerFound = false;

            for (const auto &layerProperties: availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;

        // This specifies, which extensions vulkan needs to provide to work with the currently given windowing setup.
        // This is necessary, as vulkan is platform-independent.
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData) {

        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }

    void populateCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = debugCallback,
                .pUserData = nullptr
        };
    }

    bool createInstance() {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            return false;
        }

        const auto appInfo = VkApplicationInfo{
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = "samovar",
                .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
                .pEngineName = "No Engine",
                .engineVersion = VK_MAKE_VERSION(0, 0, 1),
                .apiVersion = VK_API_VERSION_1_0
        };

        const auto extensions = getRequiredExtensions();

        // Setup debug notifications for instance creation
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        populateCreateInfo(debugCreateInfo);

        auto createInfo = VkInstanceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = enableValidationLayers ? (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo : nullptr,
                .pApplicationInfo = &appInfo,
                .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
                .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
                .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
        };

        return vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS;
    }

    bool setupDebugMessenger() {
        if (!enableValidationLayers) {
            return true;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateCreateInfo(createInfo);

        // Make sure the functions for creating and destroying the messenger are available.
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            spdlog::error("Failed to setup debug messenger.");
            return false;
        }

        return true;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        return true;
    }

    bool pickPhysicalDevice() {
        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

        if (!physicalDeviceCount) {
            spdlog::error("Could not find any vulkan-capable devices.");
            return false;
        }

        auto physicalDevices = std::vector<VkPhysicalDevice>(physicalDeviceCount);
        vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

        // Find suitable device.
        for (const auto &device : physicalDevices) {
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        return true;
    }

    bool initVulkan() {
        return createInstance() && setupDebugMessenger() && pickPhysicalDevice();
    }

    bool mainLoop() {
        while (!glfwWindowShouldClose(pWindow)) {
            glfwPollEvents();
        }

        return true;
    }

    void cleanup() {
        // Before destroying the instance we need to destroy the debug listener.
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(pWindow);
        glfwTerminate();
    }
};

int main() {
    SamovarApplication app{};

    spdlog::info("Engine started...");

    return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}