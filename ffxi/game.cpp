module;

#include <coroutine>
#include <filesystem>
#include <iostream>
#include <memory>

module ffxi;

import :game;

import :audio;
import :dat.loader;
import :entity.actor;
import :entity.landscape;
import :entity.camera.third_person_classic;
import :entity.component.equipment_test;
import :test.loader;
import :test.particle_tester;
import :system_dat;
import lotus;
import glm;
import vulkan_hpp;

FFXIGame::FFXIGame(const lotus::Settings& settings)
    : lotus::Game(settings, std::make_unique<FFXIConfig>()),
      dat_loader(std::make_unique<FFXI::DatLoader>(static_cast<FFXIConfig*>(engine->config.get())->ffxi.ffxi_install_path)),
      audio(std::make_unique<FFXI::Audio>(engine.get()))
{
}

lotus::Task<> FFXIGame::entry()
{
    default_texture = co_await lotus::Texture::LoadTexture("default", TestTextureLoader::LoadTexture, engine.get());

    system_dat = co_await SystemDat::Load(this);

    engine->lights->light.diffuse_dir = glm::normalize(-glm::vec3{-25.f, -100.f, -50.f});
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
    co_return;
}

lotus::Task<> FFXIGame::tick(lotus::time_point, lotus::duration) { co_return; }

lotus::WorkerTask<> FFXIGame::load_scene()
{
    loading_scene = std::make_unique<lotus::Scene>(engine.get());

    auto path = static_cast<FFXIConfig*>(engine->config.get())->ffxi.ffxi_install_path;
    /* zone dats vtable:
    (i < 256 ? i + 100  : i + 83635) // Model
    (i < 256 ? i + 5820 : i + 84735) // Dialog
    (i < 256 ? i + 6420 : i + 85335) // Actor
    (i < 256 ? i + 6720 : i + 86235) // Event
    */

    // auto landscape = co_await loading_scene->AddEntity<FFXILandscapeEntity>(175); //eldieme
    auto landscape = co_await loading_scene->AddEntity<FFXILandscapeEntity>(291); // reisenjima
    audio->setMusic(79, 0);
    // audio->setMusic(114, 0);
    // iroha 3111 (arciela 3074)
    // auto [player, player_components] = co_await loading_scene->AddEntity<Actor>(3111);
    auto [player, player_components] = co_await loading_scene->AddEntity<Actor>(FFXI::ActorSkeletonComponent::LookData{.look = {.race = 2,
                                                                                                                                .face = 15,
                                                                                                                                .head = 0x1000 + 64,
                                                                                                                                .body = 0x2000 + 64,
                                                                                                                                .hands = 0x3000 + 64,
                                                                                                                                .legs = 0x4000 + 64,
                                                                                                                                .feet = 0x5000 + 64,
                                                                                                                                .weapon = 0x6000 + 240,
                                                                                                                                .weapon_sub = 0x7000 + 140}});
    // fighter's lorica: ROM/93/46.DAT
    auto ac = std::get<FFXI::ActorComponent*>(player_components);
    // ac->setPos((glm::vec3(-681.f, -12.f, 161.f)), false);
    // ac->setPos((glm::vec3(419, -53.f, -103.f)), false); //eldieme entrance
    // player->setPos(glm::vec3(-430.f, -42.2f, 46.f));
    ac->setPos(glm::vec3(259.f, -87.f, 99.f), false); // reisenjima torii
    auto a = std::get<lotus::Component::AnimationComponent*>(player_components);
    auto [camera, camera_components] = co_await loading_scene->AddEntity<ThirdPersonClassicCamera>(ac, a);
    // auto ac_models = std::get<FFXI::ActorPCModelsComponent*>(player_components);
    auto ac_skeleton = std::get<FFXI::ActorSkeletonComponent*>(player_components);

    auto equip = co_await FFXI::EquipmentTestComponent::make_component(player.get(), engine.get(), *ac_skeleton);
    auto particle_tester = co_await ParticleTester::make_component(player.get(), engine.get(), *ac_skeleton);
    loading_scene->AddComponents(std::move(equip), std::move(particle_tester));

    engine->set_camera(std::get<lotus::Component::CameraComponent*>(camera_components));
    vk::Extent2D extent = engine->renderer->getExtent();
    engine->camera->setPerspective(glm::radians(70.f), extent.width / (float)extent.height, 0.01f, 1000.f);

    co_await update_scene(std::move(loading_scene));
}
