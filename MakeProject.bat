if not exist %~dp0\Build mkdir %~dp0\Build

cd %~dp0\Build

START cmake ../ -G"Visual Studio 15 Win64"