{
  description = "Client dev environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs = { self , nixpkgs ,... }: let
    system = "x86_64-linux";
  in {
    devShells."${system}".default = let
      pkgs = import nixpkgs {
        inherit system;
      };
    in pkgs.mkShell {
      packages = [
        pkgs.gcc
        pkgs.gdb
        pkgs.valgrind
        pkgs.bear
        pkgs.clang-tools
        pkgs.libGL
        
        pkgs.unzip

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
