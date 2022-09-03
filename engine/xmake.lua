add_requires("libsdl 2.0.22", "glm", "soloud", "vulkan-memory-allocator")
add_requires("cmake::Vulkan", {alias = "vulkan", system = true, configs = {link_libraries = {"Vulkan::Vulkan" }}})

target("lotus-engine")
  set_kind("static")
  set_languages("cxxlatest")
  add_packages("vulkan", "libsdl", "glm", "vulkan-memory-allocator", "soloud", {public = true})
  add_defines("GLM_FORCE_LEFT_HANDED")
  add_files("**.cpp")
  add_headerfiles("**.h")
  add_includedirs("../", {public = true})