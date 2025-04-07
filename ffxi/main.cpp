#include <SDL3/SDL_main.h>
#include <stdlib.h>

import ffxi;
import lotus;
import vulkan_hpp;

int main(int argc, char* argv[])
{
    lotus::Settings settings;
    settings.app_name = "lotus-ffxi";
    settings.app_version = vk::makeApiVersion(0, 1, 0, 0);
    FFXIGame game{settings};

    game.run();
    return EXIT_SUCCESS;
}
