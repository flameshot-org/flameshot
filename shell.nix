{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    kdePackages.qtsvg
    kdePackages.qttools
  ];
  buildInputs = with pkgs; [ kdePackages.qtbase kdePackages.kguiaddons ];
}
