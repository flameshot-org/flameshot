{ stdenv
, lib
, libsForQt5
, wrapQtAppsHook
, cmake
, qtbase
, qttools
, qtsvg 
}:

with libsForQt5;

stdenv.mkDerivation rec {
  pname = "flameshot";
  version = "0.2.0";
  
  src = ./.;

  nativeBuildInputs = [ 
    cmake 
    qttools
    qtsvg
    wrapQtAppsHook
    ];
  buildInputs = [ qtbase ];

  cmakeFlags = [
  ];

  meta = with lib; {
    homepage = "";
    description = ''
    '';
    licencse = licenses.gpl3Plus;
    platforms = with platforms; linux ++ darwin;
  };
}
