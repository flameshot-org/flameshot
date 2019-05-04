Qt wrapper library for the ZXing decoding library. 

# How to use

## To compile as dynamic library
qmake src/QZXing.pro
make

## Include the complete code to your project
In the .pro file of your project add the following line (update the path to point to the correct location of QZXing src): 
  include(../../src/QZXing.pri)
