#include <iostream>
#include "SFML/Graphics.hpp"
#include "vulkan/vulkan.hpp"

using namespace std;

int main()
{
    sf::RenderWindow window(sf::VideoMode(640, 480), "Vulkanism");
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