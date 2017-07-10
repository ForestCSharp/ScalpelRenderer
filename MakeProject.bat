if not exist %~dp0Build mkdir %~dp0Build

cd %~dp0Build

cmake ../ -G"Visual Studio 15 Win64"