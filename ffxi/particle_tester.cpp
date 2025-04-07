module;

#include <SDL3/SDL.h>
#include <chrono>
#include <coroutine>

module ffxi;

import :test.particle_tester;

import :dat.scheduler;
import :entity.component.actor_skeleton;
import :entity.component.scheduler;
import glm;
import lotus;
import vulkan_hpp;

ParticleTester::ParticleTester(lotus::Entity* _entity, lotus::Engine* _engine, FFXI::ActorSkeletonComponent& _actor)
    : Component(_entity, _engine), actor(_actor)
{
}

lotus::Task<> ParticleTester::init()
{
    // WS/dance/quarry: id + 3400
    // magic: id + 2800
    // item: id + 4912
    // ja: id + 4412
    // mobskill: id + 3900
    // pet mobskill: id + 49391
    size_t index = 2800 + 1;
    const auto& dat = static_cast<FFXIGame*>(engine->game)->dat_loader->GetDat(index);
    scheduler_resources = co_await SchedulerResources::Load(static_cast<FFXIGame*>(engine->game), dat);
}

lotus::Task<> ParticleTester::tick(lotus::time_point time, lotus::duration delta)
{
    if (add)
    {
        auto cast_scheduler = actor.getScheduler("cawh");
        engine->game->scene->AddComponents(
            co_await FFXI::SchedulerComponent::make_component(entity, engine, actor, cast_scheduler, scheduler_resources.get(), nullptr));
        start_time = time;
        add = false;
        casting = true;
    }
    if (casting && time > start_time + 3s)
    {
        if (casting_scheduler)
        {
            casting_scheduler->cancel();
            casting_scheduler = nullptr;
        }
        // engine->game->scene->component_runners->addComponent<FFXI::SchedulerComponent>(entity, actor,
        // scheduler_resources->schedulers["main"], scheduler_resources.get(), nullptr);
        casting = false;
        finished = true;
    }
    if (finished && time > start_time + 7s)
    {
        // actor.getAnimationComponent().playAnimationLoop("idl");
        finished = false;
    }
    co_return;
}

bool ParticleTester::handleInput(lotus::Input* input, const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN && event.key.repeat == 0)
    {
        switch (event.key.scancode)
        {
        case SDL_SCANCODE_R:
            add = true;
            return true;
        default:
            return false;
        }
    }
    return false;
}
