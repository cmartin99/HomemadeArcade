@echo off
pushd build\bin
call ..\buildapp.bat Template
popd
call deploy.bat