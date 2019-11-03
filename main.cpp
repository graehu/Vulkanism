#include <iostream>
#include "vulkan/vulkan.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifndef NDEBUG
#define NDEBUG 0
#pragma warning("NDEBUG Didn't exist priror to compile!!")
#endif

class Renderer
{
private:
    void InitWindow();
    void InitVulkan();
    bool CheckValidationLaterSupport();
    void CreateInstance();
    void MainLoop();
    void Cleanup();
    //
    GLFWwindow* window = nullptr;
    VkInstance instance;
public:
    Renderer();
    ~Renderer();
    void run()
    {
        InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }
    const unsigned int window_width = 400;
    const unsigned int window_height = 300;
    const bool enable_validation_layers = NDEBUG ? false: true;
    const std::vector<const char*> validation_layers = 
    {
        // I appear to have very old vulkan libs or something
        // The below line should be valid. 
        // "VK_LAYER_KHRONOS_validation"
        "VK_LAYER_LUNARG_standard_validation"
    };

};
bool Renderer::CheckValidationLaterSupport()
{
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());
    for (const char* layer_name : validation_layers)
    {
        std::cout << "finding: " << layer_name << std::endl;
        bool layer_found = false;
        for(const auto& layer_properties : available_layers)
        {
            std::cout << "\tcomparing: " << layer_properties.layerName << std::endl;
            if(strcmp(layer_name, layer_properties.layerName) == 0)
            {
                layer_found = true;
                break;
            }
        }
        if(layer_found == false)
        {
            std::cout << "RUH ROH" << std::endl;
            return false;
        }
    }
    return true;
}
Renderer::Renderer() { }
Renderer::~Renderer() { }

void Renderer::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(window_width, window_height, "Vulkanism", nullptr, nullptr);
}
void Renderer::InitVulkan()
{
    CreateInstance();
}
void Renderer::CreateInstance()
{
    if (enable_validation_layers && !CheckValidationLaterSupport())
    {
        throw std::runtime_error("validation layers requested, but not supported!");
    }
    //app info
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Vulkanism";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;
    //create info
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    if(enable_validation_layers)
    {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    //
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> required_extensions(glfwExtensions, glfwExtensions+glfwExtensionCount);
    if(enable_validation_layers)
    {
        required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    create_info.enabledExtensionCount = static_cast<uint32_t>(required_extensions.size());
    create_info.ppEnabledExtensionNames = required_extensions.data();
    create_info.enabledLayerCount = 0;

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "available extensions:" << std::endl;
    for (const auto& extension : extensions)
    {
        std::cout << "\t" << extension.extensionName << std::endl;
    }
    //VK_KHR_wayland_surface is supported, consider using that.

    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create vk instance!");
    }

}
void Renderer::MainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
      glfwPollEvents();
    }  
}
void Renderer::Cleanup()
{
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}



int main()
{
    Renderer renderer;
    try
    {
        renderer.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}