#pragma once

#include <filesystem>
import lotus;

class FFXIConfig : public lotus::Config
{
public:
    struct FFXIInfo
    {
        std::filesystem::path ffxi_install_path;
    } ffxi{};

    FFXIConfig();
};
