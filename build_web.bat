@echo off

if not defined EMSDK (
	pushd C:\emsdk
	call emsdk_env.bat
	popd
)

@echo on
emcc main.cpp -o main.html -I. -s USE_SDL=2 -s FULL_ES3=1 -s DISABLE_EXCEPTION_CATCHING=0
@echo off
