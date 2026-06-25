{
  description = "infoch";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    {
      self,
      nixpkgs,
      ...
    }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
    {
      inherit system;
      devShells.${system}.default =
        pkgs.mkShell.override
          {
            stdenv = pkgs.llvmPackages_22.stdenv;
          }
          {
            packages = with pkgs; [
              git
              llvmPackages_22.clang-tools
              gdb
              lldb
              include-what-you-use
              neocmakelsp

              cmake
              ninja
              llvmPackages_22.clang
              gcc
              ccache
              llvmPackages_22.bintools
              mold
              pkg-config

              imagemagick
              lua
              jemalloc
              fmt
            ];
          };
    };
}
