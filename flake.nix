{
  description = "DEVS Simulation Environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  };

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux";
    pkgs = nixpkgs.legacyPackages.${system};
    cadmium-src = pkgs.fetchgit {
      url = "https://github.com/SimulationEverywhere/cadmium_v2.git";
      rev = "f0f3ed248f4f847b819a0e07e5865136b1d23cc9";
      hash = "sha256-Fc/UyT9dPJ/xxSdAzgpZgfbPlP8K2J4O+UEH0K2a3+M=";
      fetchSubmodules = true;
    };
  in {
    devShells.${system}.default = pkgs.mkShell.override {stdenv = pkgs.clangStdenv;} {
      nativeBuildInputs = builtins.attrValues {
        inherit
          (pkgs)
          nixd
          alejandra
          cmake
          clang-tools
          uv
          python3
          ;
      };

      buildInputs = builtins.attrValues {
        inherit (pkgs) boost;
      };

      shellHook = ''
        export UV_PYTHON_DOWNLOADS="never"
        export UV_PYTHON="${pkgs.python3}/bin/python3"
        export CADMIUM_ROOT="${cadmium-src}"
        uv sync --project orchestration
        export PATH="$PWD/orchestration/.venv/bin:$PATH"
      '';
    };
  };
}
