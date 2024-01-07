cmake_minimum_required(VERSION 3.10)

project(samovar)

add_executable(samovar src/main.cpp)

# The environment variables are provided by the nix-shell context that we are in.
#target_include_directories(samovar PRIVATE ${VULKAN_INCLUDE_PATH} ${GLFW_INCLUDE_PATH} ${SPDLOG_INCLUDE_PATH})

# Make sure that spdlog works, with the nix setup you have to add this compile time flag for the headers to work...
#target_compile_definitions(samovar PRIVATE SPDLOG_FMT_EXTERNAL)

# The cmake thing is prebundled with packages to find, so you HAVE to use the one from the nix store.....
find_package(spdlog REQUIRED)

# Why can it just find them, does nix somehow tamper with PKG_CONFIG_PATH or LD_LIBRARY_PATH or sth...
target_link_libraries(samovar vulkan glfw spdlog::spdlog)