#include "GameGUI.h"
#include <iostream>

int main()
{
    try
    {
        GameGUI game;
        game.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
