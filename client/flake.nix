{
  description = "Client dev environment";

  nixConfig = {
    allow-impure = true;
    allow-unfree = true;
  };

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs = { self , nixpkgs ,... }: let
    system = "x86_64-linux";
  in {
    devShells."${system}".default = let
      pkgs = import nixpkgs {
        inherit system;
        config = {
          allowUnfree = true;
          # Explicitly allow Steam
          allowUnfreePredicate = pkg: builtins.elem (pkgs.lib.getName pkg) [
            "steam"
            "steam-original"
            "steam-runtime"
          ];
        };
      };
    in pkgs.mkShell {
      packages = [
        pkgs.blender
        pkgs.steam
        pkgs.gcc
        pkgs.gdb
        pkgs.valgrind
        pkgs.bear
        pkgs.clang-tools
        pkgs.libGL
        pkgs.cmake

        # X11 dependencies
        pkgs.xorg.libX11
        pkgs.xorg.libX11.dev
        pkgs.xorg.libXcursor
        pkgs.xorg.libXi
        pkgs.xorg.libXinerama
        pkgs.xorg.libXrandr
      ];
    };
  };
}
