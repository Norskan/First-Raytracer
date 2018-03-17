@echo off

REM setup dev env
if not defined devEnvSet (
	call vcvarsall.bat x64
    
	set devEnvSet=1
)

set nameExe=RayTracer
set runTree=.\..\run_tree
set copyflags=/b/v/y 

REM clean directories
rmdir /s/q bin
mkdir bin

del /f .\run_tree\*.exe >NUL
del /f .\run_tree\*.pdb >NUL

pushd .\bin


.\..\tools\ctime.exe -begin  build_timing.ctm

REM compiler flags

set compilerFlags= -EHsc -O2 -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -Z7
cl %compilerFlags% -Fe%nameExe% ./../src/ray_main.cpp /link 

copy %copyflags%  %nameExe%.exe %runTree% >NUL

copy %copyflags% *.pdb %runTree% >NUL

.\..\tools\ctime.exe -end build_timing.ctm

popd
REM run.bat

@echo DONE!

