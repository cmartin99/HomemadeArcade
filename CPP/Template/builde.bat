@echo off
pushd build
del bin /q
xcopy libs bin /c /q
cd bin
call ..\buildeng.bat
popd
call buildsh.bat
