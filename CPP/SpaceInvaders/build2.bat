@echo off
pushd build
del bin /q
xcopy libs bin /c /q
cd bin
call ..\buildeng.bat
call ..\buildapp.bat SpaceInvaders
popd
call buildsh.bat
call deploy.bat