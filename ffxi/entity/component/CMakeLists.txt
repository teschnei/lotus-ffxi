target_sources(ffxi PUBLIC
    FILE_SET CXX_MODULES
    FILES
    actor.cppm
    actor_skeleton.cppm
    camera_third_person.cppm
    input_third_person_classic.cppm
    input_third_person_modern.cppm
    equipment_test.cppm
    generator.cppm
    generator_light.cppm
    landscape.cppm
    particle.cppm
    scheduler.cppm
    PRIVATE
    actor.cpp
    actor_skeleton.cpp
    camera_third_person.cpp
    input_third_person_classic.cpp
    input_third_person_modern.cpp
    equipment_test.cpp
    generator.cpp
    generator_light.cpp
    landscape.cpp
    particle.cpp
    scheduler.cpp
)
