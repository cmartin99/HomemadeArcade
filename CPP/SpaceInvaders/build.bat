@echo off
pushd build\bin
call ..\buildapp.bat SpaceInvaders
popd
call deploy.bat