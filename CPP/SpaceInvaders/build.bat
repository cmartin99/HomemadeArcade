@echo off
pushd build\bin
call ..\buildapp.bat
REM call ..\buildplugins.bat
popd
call deploy.bat