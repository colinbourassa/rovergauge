@echo off
call cmake . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto handleerror
mingw32-make install
if errorlevel 1 goto handleerror
mingw32-make package
goto done
:handleerror
echo Error, aborting build.
:done
pause