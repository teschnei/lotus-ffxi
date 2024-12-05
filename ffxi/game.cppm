module;

#include <memory>

export module ffxi:game;

import :audio;
import :config;
import :dat.loader;
import :system_dat;
import lotus;

export class FFXIGame : public lotus::Game
{
public:
    FFXIGame(const lotus::Settings& settings);

    virtual lotus::Task<> entry() override;
    virtual lotus::Task<> tick(lotus::time_point, lotus::duration) override;

    std::unique_ptr<FFXI::DatLoader> dat_loader;
    std::unique_ptr<SystemDat> system_dat;
    std::unique_ptr<FFXI::Audio> audio;

private:
    std::shared_ptr<lotus::Texture> default_texture;
    std::unique_ptr<lotus::Scene> loading_scene;

    lotus::WorkerTask<> load_scene();
};
