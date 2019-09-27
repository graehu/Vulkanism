#include <iostream>
#include "SFML/Graphics.hpp"
#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

using namespace std;

int main()
{
    sf::RenderWindow window(sf::VideoMode(640, 480), "Vulkanism");
    sf::WindowHandle handle = window.getSystemHandle();
    sf::ContextSettings settings = window.getSettings();
    VkInstance instance;
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
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        window.clear();
        window.display();
    }
    return 0;
}