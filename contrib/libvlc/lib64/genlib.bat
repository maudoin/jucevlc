
SET buildtoolsbin="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC\14.37.32822\bin\Hostx64\x64"
SET dumpbin=%buildtoolsbin%\dumpbin.exe
SET lib=%buildtoolsbin%\lib.exe

REM @echo off

REM Usage: dll2lib input.dll [32|64]
REM
REM Generates some-file.lib from some-file.dll, making an intermediate
REM some-file.def from the results of dumpbin /exports some-file.dll.
REM
REM Requires 'dumpbin' and 'lib' in PATH - run from VS developer prompt.
REM 
REM Script inspired by http://stackoverflow.com/questions/9946322/how-to-generate-an-import-library-lib-file-from-a-dll
SETLOCAL
if "%2"=="32" (SET machine=x86) else (SET machine=x64)
SET batchpath=%~dp0
SET dll_file=%1
SET dll_file_no_ext=%~n1
SET exports_file=%batchpath%/%dll_file_no_ext%-exports.txt
SET def_file=%batchpath%/%dll_file_no_ext%.def
SET lib_file=%batchpath%/%dll_file_no_ext%.lib
SET lib_name=%dll_file_no_ext%

%dumpbin% /exports %dll_file% > %exports_file%

echo LIBRARY %lib_name% > %def_file%
echo EXPORTS >> %def_file%
for /f "skip=19 tokens=1,4" %%A in (%exports_file%) do if NOT "%%B" == "" (echo %%B @%%A >> %def_file%)

%lib% /def:%def_file% /out:%lib_file% /machine:%machine%

REM Clean up temporary intermediate files
del %exports_file% %def_file% %dll_file_no_ext%.exp