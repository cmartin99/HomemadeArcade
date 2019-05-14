@echo off
pushd deploy
del *.log /q
win32_template.exe
popd
