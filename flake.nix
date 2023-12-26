{
  description = "samovar's decription";
  inputs = {
    nixpkgs.url = "nixpkgs/nixos-23.11";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = inputs@{ self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        overlays = [ ];
        pkgs = import nixpkgs {
          inherit system overlays;
          config.allowBroken = true;
        };
      in {
        devShell =
          pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
             packages = with pkgs;
                [ vulkan-loader
                  vulkan-validation-layers
                  vulkan-headers
                  glfw
                  shaderc ];

             shellHook = with pkgs; ''
                export VULKAN_INCLUDE_PATH=${vulkan-headers}/include
                export VULKAN_LIBRARY_PATH=${vulkan-loader}/lib
                export GLFW_INCLUDE_PATH=${glfw}/include
                export GLFW_LIBRARY_PATH=${glfw}/lib
             '';
          };
      });
}
