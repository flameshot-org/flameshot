@echo off
REM 设置 VS2022 的开发者命令行环境
CALL "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM 创建构建目录
IF NOT EXIST build_x64_release (
    mkdir build_x64_release
)

cd build_x64_release

REM 使用 CMake 生成 Release 版解决方案
cmake -DCMAKE_BUILD_TYPE=Release -A x64 ..

REM 编译
cmake --build . --config Release
REM 调用PACKAGE工程进行一键打包
cmake --build . --config Release --target PACKAGE

cd ..
echo 编译打包完成！
pause