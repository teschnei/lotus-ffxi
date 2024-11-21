#pragma once

#include <filesystem>
#include <lotus/config.h>

class FFXIConfig : public lotus::Config
{
public:
    struct FFXIInfo
    {
        std::filesystem::path ffxi_install_path;
    } ffxi{};

    FFXIConfig();
};
