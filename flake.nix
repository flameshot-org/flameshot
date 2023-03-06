{
  description = "Flameshot flake build";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/22.11";
    
    utils.url = "github:numtide/flake-utils";
    utils.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs = { self, nixpkgs, ... }@inputs: inputs.utils.lib.eachSystem [
    "x86_64-linux" "i686-linux" "aarch64-linux" "x86_64-darwin"
  ] (system: let pkgs = import nixpkgs {
                   inherit system;
                 };
             in {
               devShell = pkgs.mkShell rec {
                 name = "flameshot";

                 packages = with pkgs; [
                   # Development Tools
                   cmake
                   cmakeCurses
                   libsForQt5.qt5.qttools
                   qt5.wrapQtAppsHook

                   # Dependencies
                   libsForQt5.qt5.qtsvg
                   libsForQt5.qt5.qtbase
                 ];
               };
               defaultPackage = pkgs.callPackage ./default.nix {
                qtbase = pkgs.libsForQt5.qt5.qtbase;
                qttools = pkgs.libsForQt5.qt5.qttools;
                qtsvg = pkgs.libsForQt5.qt5.qtsvg;
                wrapQtAppsHook = pkgs.qt5.wrapQtAppsHook;
                };
             });
}