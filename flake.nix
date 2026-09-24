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
      url = "https://github.com/SimulationEverywhere/cadmium.git";
      rev = "72a11341aa684010caf1ca5dee779f0e7e84dfe9";
      hash = "sha256-NXCEULBILIg5pO0o6YO6XuWVnC31akOrHu7NDzXaSx4=";
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
      '';
    };
  };
}
