{ pkgs ? import <nixpkgs> { } }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    qt5.full
    qt5.qttools
    qt5.qtsvg
  ];
  buildInputs = with pkgs; [ qt5.qtbase libsForQt5.kguiaddons ];
}
