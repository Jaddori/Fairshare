@echo off

IF NOT EXIST w:\build\ mkdir w:\build\
pushd w:\build\
cl.exe -nologo -Zi -EHsc -Fefairshare.exe v:\main.cpp /DWIN32 /link /INCREMENTAL:NO ws2_32.lib
popd