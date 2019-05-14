@echo off
if not defined VisualStudioVersion (
   call "D:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\vsdevcmd" -arch=x64
)
