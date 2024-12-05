module;

#include <filesystem>
#ifdef _WIN32
#include <Windows.h>
#include <winreg.h>
#endif

module ffxi:config;

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

FFXIConfig::FFXIConfig() : lotus::Config()
{
    if (ffxi.ffxi_install_path.empty())
    {
#ifdef _WIN32
        auto keys = {R"(SOFTWARE\PlayOnline\InstallFolder)", R"(SOFTWARE\PlayOnlineUS\InstallFolder)", R"(SOFTWARE\PlayOnlineEU\InstallFolder)"};
        std::string value;
        uint32_t size = 512;
        value.resize(size);
        for (const auto& key : keys)
        {
            auto res = RegGetValue(HKEY_LOCAL_MACHINE, key, "0001", RRF_RT_REG_SZ | RRF_SUBKEY_WOW6432KEY, nullptr, (void*)value.data(), (LPDWORD)&size);
            if (res == ERROR_SUCCESS)
            {
                value.resize(size - 1);
                ffxi.ffxi_install_path = value;
            }
        }
#else
        const char* path = getenv("FFXI_PATH");
        if (path)
        {
            ffxi.ffxi_install_path = path;
        }
#endif
    }
}
