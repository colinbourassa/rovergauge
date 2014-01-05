@echo off
cmake . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release .
mingw32-make package
pause