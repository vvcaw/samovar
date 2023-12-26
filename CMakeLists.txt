cmake_minimum_required(VERSION 3.10)

project(samovar)

add_executable(samovar src/main.cpp)

# The environment variables are provided by the nix-shell context that we are in.
target_include_directories(samovar PRIVATE ${VULKAN_INCLUDE_PATH} ${GLFW_INCLUDE_PATH})

# Why can it just find them, does nix somehow tamper with PKG_CONFIG_PATH or LD_LIBRARY_PATH or sth...
target_link_libraries(samovar vulkan glfw)