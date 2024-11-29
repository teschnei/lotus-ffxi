#pragma once

#include "dat/dat.h"
#include "scheduler_resources.h"
#include <lotus/renderer/sdl_inc.h>
#include <memory>

import lotus;

namespace FFXI
{
class Generator;
class Keyframe;
class Scheduler;
class ActorSkeletonComponent;
class SchedulerComponent;
} // namespace FFXI

class ParticleTester : public lotus::Component::Component<ParticleTester>
{
public:
    explicit ParticleTester(lotus::Entity*, lotus::Engine* engine, FFXI::ActorSkeletonComponent& actor);
    lotus::Task<> init();
    lotus::Task<> tick(lotus::time_point time, lotus::duration delta);
    bool handleInput(lotus::Input*, const SDL_Event&);

private:
    FFXI::ActorSkeletonComponent& actor;
    std::vector<std::shared_ptr<lotus::Model>> models;
    std::unique_ptr<SchedulerResources> scheduler_resources;
    std::unordered_map<std::string, std::shared_ptr<lotus::Texture>> texture_map;
    bool add{false};
    bool casting{false};
    bool finished{false};
    FFXI::SchedulerComponent* casting_scheduler{nullptr};
    lotus::time_point start_time;
};
