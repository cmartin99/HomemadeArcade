cl /c /nologo /Zi /EHa- /Ob1 /Oi /FC /GR- /WX /W4 /wd4091 /wd4577 /wd4530 /wd4477 /wd4244 /wd4100 /wd4189 /wd4459 /wd4324 /wd4316 /wd4238 /wd4239 /wd4146 /wd4005 /wd4996 /wd4267 /wd4458 /wd4201 /wd4700 /wd4702 /D_WIN32_WINNT=0x0501 /D_AMD64_ /D_PROFILE_ /DVK_USE_PLATFORM_WIN32_KHR /I \CPP\%1\source\code\ /I \CPP\%1\source\code\shaders /I \CPP\%1\source\code\platform\win32 /I \CPP\engine\code /I \CPP\engine\code\vulkan /I \CPP\include /I \CPP\boost /I \CPP\include\vulkan /I "C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include" \CPP\%1\source\code\%1.cpp

cl /nologo /Zi /EHa- /Ob1 /Oi /FC /GR- /WX /W4 /wd4091 /wd4577 /wd4530 /wd4477 /wd4244 /wd4100 /wd4189 /wd4459 /wd4324 /wd4316 /wd4238 /wd4146 /wd4005 /wd4996 /wd4267 /wd4458 /wd4201 /wd4700 /wd4702 /D_WIN32_WINNT=0x0501 /D_AMD64_ /D_PROFILE_ /DVK_USE_PLATFORM_WIN32_KHR /I \CPP\%1\source\code\ /I \CPP\%1\source\code\shaders /I \CPP\%1\source\code\platform\win32 /I \CPP\engine\code /I \CPP\engine\code\vulkan /I \CPP\include /I \CPP\boost /I \CPP\include\vulkan /I "C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include" /I "D:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\crt\src\vcruntime" \CPP\%1\source\code\platform\win32\Win32_%1.cpp user32.lib gdi32.lib comdlg32.lib TdEngine.obj vulkan-1.lib %1.obj Ws2_32.lib