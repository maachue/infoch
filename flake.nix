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
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          git
          clang-tools
          gdb
          lldb
          neocmakelsp

          cmake
          ninja
          clang
          gcc
          ccache
          llvmPackages.bintools
          mold
          pkg-config

          imagemagick
          jemalloc
          fmt
        ];
      };
    };
}
