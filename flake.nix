{
  description = "A Wayland-first launcher in QtQuick + C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
      pkgsFor = system: import nixpkgs { inherit system; };
    in
    {
      packages = forAllSystems (system:
        let
          pkgs = pkgsFor system;
        in
        {
          default = pkgs.stdenv.mkDerivation {
            pname = "awelaunch";
            version = "0.1.0";
            src = ./.;

            nativeBuildInputs = with pkgs; [
              pkg-config
              cmake
              ninja
              qt6.wrapQtAppsHook
            ];

            buildInputs = with pkgs; [
              qt6.qtbase
              qt6.qtdeclarative
              qt6.qtwayland
              wayland
              wayland-protocols
              yaml-cpp
            ];
          };
        }
      );

      devShells = forAllSystems (system:
        let
          pkgs = pkgsFor system;
        in
        {
          default = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              pkg-config
              cmake
              ninja
              qt6.wrapQtAppsHook
              go-task
            ];

            buildInputs = with pkgs; [
              qt6.qtbase
              qt6.qtdeclarative
              qt6.qtwayland
              wayland
              wayland-protocols
              yaml-cpp
            ];

            shellHook = ''
              export QT_QPA_PLATFORM=wayland
              export QML2_IMPORT_PATH=${pkgs.qt6.qtdeclarative}/lib/qt-6/qml:${pkgs.qt6.qtwayland}/lib/qt-6/qml
              export QT_PLUGIN_PATH=${pkgs.qt6.qtbase}/lib/qt-6/plugins:${pkgs.qt6.qtwayland}/lib/qt-6/plugins
              echo "Awelauncher Dev Shell"
            '';
          };
        }
      );
    };
}
