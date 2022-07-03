{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    qt5.full
    qt5.qttools
    qt5.qtsvg
  ];
  buildInputs = [ pkgs.qt5.qtbase ];
}
