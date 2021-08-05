@echo off
setlocal

rem set ccflags=/nologo /FC /Od /Zi /EHa- /Gm- /GR- /GS- /Gs9999999
rem set ldflags=/nodefaultlib /subsystem:console /stack:0x100000,0x100000
rem set libs=kernel32.lib

set ccflags=/nologo /FC /Od /Zi /EHa- /Gm- /DCOMPILER_MSVC
set ldflags=
set libs=
set exename=win32_main

if not exist build mkdir build

pushd build

cd

cl %ccflags% ..\source\win32_main.cpp /Fe:%exename%.exe /link %ldflags% %libs%

if %errorlevel% neq 0 goto :end
%exename%.exe
start out.bmp
start tree.bmp
:end

popd
