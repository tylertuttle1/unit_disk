@echo off
setlocal

rem set ccflags=/nologo /FC /Od /Zi /EHa- /Gm- /GR- /GS- /Gs9999999
rem set ldflags=/nodefaultlib /subsystem:console /stack:0x100000,0x100000
rem set libs=kernel32.lib

set ccflags=/nologo /FC /Od /Zi /EHa- /Gm-
set ldflags=
set libs=
set exename=win32_main

pushd build

cl %ccflags% ..\source\win32_main.cpp /Fe:%exename%.exe /link %ldflags% %libs%

if %errorlevel% neq 0 goto :end
%exename%.exe
rem start tree.bmp
start out.bmp
:end

popd
