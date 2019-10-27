#include <iostream>
#include "vulkan/vulkan.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
using namespace std;

class Renderer
{
private:
    void InitWindow();
    void InitVulkan();
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
};

Renderer::Renderer() { }
Renderer::~Renderer() { }

void Renderer::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(400, 300, "Vulkanism", nullptr, nullptr);
}
void Renderer::InitVulkan()
{
    CreateInstance();
}
void Renderer::CreateInstance()
{
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
    //
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    create_info.enabledExtensionCount = glfwExtensionCount;
    create_info.ppEnabledExtensionNames = glfwExtensions;
    create_info.enabledLayerCount = 0;
    VkResult resault = vkCreateInstance(&create_info, nullptr, &instance);
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