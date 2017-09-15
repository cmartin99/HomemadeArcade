@echo off
pushd deploy
del *.log /q
win32_spaceinvaders.exe
popd
