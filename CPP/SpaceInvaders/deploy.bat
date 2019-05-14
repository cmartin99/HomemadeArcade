@echo off
xcopy \cpp\engine\content deploy\content /C /Q /Y /E
xcopy source\content deploy\content /C /Q /Y /E
xcopy \cpp\engine\code\shaders\*.spv deploy\content\shaders /C /Q /Y /E
xcopy source\code\shaders\*.spv deploy\content\shaders /C /Q /Y /E
copy build\bin\win32_spaceinvaders.exe deploy
echo deployed to \cpp\spaceinvaders\deploy