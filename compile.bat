
@echo on

if exist setenv.bat call setenv.bat


:initialize
if not exist "build\mingw" mkdir build\mingw
if not exist "build\vs2010"  mkdir build\vs2010


:compile_mingw
pushd build\mingw
call cmake ..\.. -G "MinGW Makefiles" -DQT_QMAKE_EXECUTABLE="%QT%\mingw\bin\qmake.exe" -DCMAKE_INSTALL_PREFIX="../package" -DENABLE_STASM=ON -DCMAKE_BUILD_TYPE=Release
call mingw32-make -j3
call mingw32-make install
popd


:final
pause
