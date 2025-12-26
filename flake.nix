{
  description = "Run in awe! - A Wayland-first launcher in QtQuick + C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
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
            pname = "awelauncher";
            version = nixpkgs.lib.removeSuffix "\n" (builtins.readFile ./VERSION);
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
              kdePackages.layer-shell-qt
              wayland
              wayland-protocols
              yaml-cpp
            ];

            qtWrapperArgs = [
              "--prefix QML2_IMPORT_PATH : ${pkgs.kdePackages.layer-shell-qt}/lib/qt-6/qml"
              "--prefix XDG_DATA_DIRS : ${placeholder "out"}/share"
            ];
          };
          awelauncher = self.packages.${system}.default;
        }
      );

      apps = forAllSystems (system: {
        awelaunch = {
          type = "app";
          program = "${self.packages.${system}.default}/bin/awelaunch";
        };
        default = self.apps.${system}.awelaunch;
      });

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
              doxygen
            ];

            buildInputs = with pkgs; [
              qt6.qtbase
              qt6.qtdeclarative
              qt6.qtwayland
              kdePackages.layer-shell-qt
              wayland
              wayland-protocols
              yaml-cpp
            ];

            shellHook = ''
              export QT_QPA_PLATFORM=wayland
              export QML2_IMPORT_PATH=${pkgs.qt6.qtdeclarative}/lib/qt-6/qml:${pkgs.qt6.qtwayland}/lib/qt-6/qml:${pkgs.kdePackages.layer-shell-qt}/lib/qt-6/qml
              export QT_PLUGIN_PATH=${pkgs.qt6.qtbase}/lib/qt-6/plugins:${pkgs.qt6.qtwayland}/lib/qt-6/plugins:${pkgs.kdePackages.layer-shell-qt}/lib/qt-6/plugins
              echo "Awelauncher Dev Shell"
            '';
          };
        }
      );
      
      checks = forAllSystems (system:
        let
          pkgs = pkgsFor system;
        in
        {
          e2e_test = pkgs.stdenv.mkDerivation {
            name = "awelauncher-e2e-test";
            src = ./tests/e2e;
            
            buildInputs = with pkgs; [
              weston
              grim
              bash
              self.packages.${system}.default # awelauncher
            ];
            
            # Needed for Qt to run in the check environment
            nativeBuildInputs = [ pkgs.qt6.wrapQtAppsHook ];
            
            dontBuild = true;
            
            installPhase = ''
              mkdir -p $out
              cp run_headless.sh $out/test.sh
              chmod +x $out/test.sh
            '';
            
            # Run the test
            doCheck = true;
            checkPhase = ''
              export QT_QPA_PLATFORM=wayland
              # Environment setup for Qt
              export QML2_IMPORT_PATH=${pkgs.qt6.qtdeclarative}/lib/qt-6/qml:${pkgs.qt6.qtwayland}/lib/qt-6/qml:${pkgs.kdePackages.layer-shell-qt}/lib/qt-6/qml
              export QT_PLUGIN_PATH=${pkgs.qt6.qtbase}/lib/qt-6/plugins:${pkgs.qt6.qtwayland}/lib/qt-6/plugins:${pkgs.kdePackages.layer-shell-qt}/lib/qt-6/plugins
              
              ./run_headless.sh
              touch $out/passed
            '';
          };
        }
      );
    };
}
