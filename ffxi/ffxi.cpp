#include "ffxi.h"

#include "test_loader.h"
#include "particle_tester.h"
#include "audio/ffxi_audio.h"
#include "entity/landscape_entity.h"
#include "entity/actor.h"
#include "entity/component/third_person_ffxi_entity_input.h"
#include "entity/component/third_person_ffxiv_entity_input.h"
#include "entity/component/equipment_test_component.h"
#include "entity/third_person_ffxi_camera.h"
#include "entity/third_person_ffxiv_camera.h"

#include "engine/scene.h"
#include "engine/ui/element.h"
#include "engine/entity/free_flying_camera.h"
#include "engine/entity/component/camera_cascades_component.h"

#include "engine/entity/component/animation_component.h"
#include "engine/entity/component/component_rewrite_test/deformable_raster_component.h"
#include "engine/entity/component/component_rewrite_test/deformable_raytrace_component.h"

#include "engine/entity/component/component_rewrite_test/instanced_raster_component.h"
#include "engine/entity/component/component_rewrite_test/instanced_raytrace_component.h"

#include <iostream>

FFXIGame::FFXIGame(const lotus::Settings& settings) : lotus::Game(settings, std::make_unique<FFXIConfig>()),
    dat_loader(std::make_unique<FFXI::DatLoader>(static_cast<FFXIConfig*>(engine->config.get())->ffxi.ffxi_install_path)),
    audio(std::make_unique<FFXI::Audio>(engine.get()))
{
}

lotus::Task<> FFXIGame::entry() 
{
    default_texture = co_await lotus::Texture::LoadTexture("default", TestTextureLoader::LoadTexture, engine.get());

    system_dat = co_await SystemDat::Load(this);

    engine->lights->light.diffuse_dir = glm::normalize(-glm::vec3{ -25.f, -100.f, -50.f });
    auto ele = std::make_shared<lotus::ui::Element>();
    ele->SetPos({20, -20});
    ele->SetHeight(300);
    ele->SetWidth(700);
    ele->anchor = lotus::ui::Element::AnchorPoint::BottomLeft;
    ele->parent_anchor = lotus::ui::Element::AnchorPoint::BottomLeft;
    ele->bg_colour = glm::vec4{0.f, 0.f, 0.f, 0.4f};
    co_await engine->ui->addElement(ele);

    auto ele2 = std::make_shared<lotus::ui::Element>();
    ele2->SetPos({10, -10});
    ele2->SetHeight(30);
    ele2->SetWidth(ele->GetWidth() - 20);
    ele2->anchor = lotus::ui::Element::AnchorPoint::BottomLeft;
    ele2->parent_anchor = lotus::ui::Element::AnchorPoint::BottomLeft;
    ele2->bg_colour = glm::vec4{0.f, 0.f, 0.f, 0.7f};
    co_await engine->ui->addElement(ele2, ele);

    engine->worker_pool->background(load_scene());
}

lotus::Task<> FFXIGame::tick(lotus::time_point, lotus::duration)
{
    co_return;
}

lotus::WorkerTask<> FFXIGame::load_scene()
{
    loading_scene = std::make_unique<lotus::Scene>(engine.get());
    loading_scene->component_runners->registerComponent<lotus::Test::AnimationComponent>();
    loading_scene->component_runners->registerComponent<lotus::Test::PhysicsComponent>();
    loading_scene->component_runners->registerComponent<lotus::Test::DeformedMeshComponent>();
    loading_scene->component_runners->registerComponent<lotus::Test::DeformableRasterComponent>();
    loading_scene->component_runners->registerComponent<lotus::Test::DeformableRaytraceComponent>();
    loading_scene->component_runners->registerComponent<lotus::Test::InstancedModelsComponent>();
    loading_scene->component_runners->registerComponent<lotus::Test::InstancedRasterComponent>();
    loading_scene->component_runners->registerComponent<lotus::Test::InstancedRaytraceComponent>();

    auto path = static_cast<FFXIConfig*>(engine->config.get())->ffxi.ffxi_install_path;
    /* zone dats vtable:
    (i < 256 ? i + 100  : i + 83635) // Model
    (i < 256 ? i + 5820 : i + 84735) // Dialog
    (i < 256 ? i + 6420 : i + 85335) // Actor
    (i < 256 ? i + 6720 : i + 86235) // Event
    */

    auto landscape = co_await loading_scene->AddEntity<FFXILandscapeEntity>(291, loading_scene.get());
    audio->setMusic(79, 0);
    //iroha 3111 (arciela 3074)
    //auto player = co_await loading_scene->AddEntity<Actor>(3111);
    auto player = co_await loading_scene->AddEntity<Actor>(LookData{ .look = {
        .race = 2,
        .face = 15,
        .head = 0x1000 + 65,
        .body = 0x2000 + 65,
        .hands = 0x3000 + 65,
        .legs = 0x4000 + 65,
        .feet = 0x5000 + 65,
        .weapon = 0x6000 + 140,
        .weapon_sub = 0x7000 + 140
        }
    });
    //player->setPos(glm::vec3(-430.f, -42.2f, 46.f));
    player->setPos(glm::vec3(259.f, -87.f, 99.f));
    auto camera = co_await loading_scene->AddEntity<ThirdPersonFFXIVCamera>(std::weak_ptr<lotus::Entity>(player));
    if (engine->config->renderer.render_mode == lotus::Config::Renderer::RenderMode::Rasterization)
    {
        co_await camera->addComponent<lotus::CameraCascadesComponent>();
    }
    engine->set_camera(camera.get());
    engine->camera->setPerspective(glm::radians(70.f), engine->renderer->swapchain->extent.width / (float)engine->renderer->swapchain->extent.height, 0.01f, 1000.f);
    engine->camera->setPerspective(glm::radians(70.f), engine->renderer->swapchain->extent.width / (float)engine->renderer->swapchain->extent.height, 0.01f, 1000.f);

    co_await player->addComponent<ThirdPersonEntityFFXIVInputComponent>(engine->input.get());
    co_await player->addComponent<ParticleTester>(engine->input.get());
    co_await player->addComponent<EquipmentTestComponent>(engine->input.get());

    auto& old_skelly = *player->getComponent<lotus::AnimationComponent>()->skeleton;
    auto new_skelly = std::make_unique<lotus::Skeleton>();

    for (auto& b : old_skelly.bones)
    {
        new_skelly->bones.emplace_back(b.parent_bone, b.rot, b.trans);
    }

    for (auto& [a, b] : old_skelly.animations)
    {
        new_skelly->animations.emplace(a, std::make_unique<lotus::Animation>(*b));
    }

    auto a = loading_scene->component_runners->addComponent<lotus::Test::AnimationComponent>(player.get(), std::move(new_skelly));
    auto p = co_await loading_scene->component_runners->addComponent<lotus::Test::PhysicsComponent>(player.get());
    auto d = co_await loading_scene->component_runners->addComponent<lotus::Test::DeformedMeshComponent>(player.get(), *a, player->models);
    if (engine->config->renderer.RasterizationEnabled())
        auto r = loading_scene->component_runners->addComponent<lotus::Test::DeformableRasterComponent>(player.get(), *d, *p);
    if (engine->config->renderer.RaytraceEnabled())
    auto rt = co_await loading_scene->component_runners->addComponent<lotus::Test::DeformableRaytraceComponent>(player.get(), *d, *p);

    co_await update_scene(std::move(loading_scene));

}