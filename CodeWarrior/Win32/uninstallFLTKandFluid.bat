@echo off

echo Uninstall FLTK and Fluid for Metrowerks' CodeWarrior
echo .

echo Deleting FLTK Libararies and Includes
if exist "%CWFOLDER%\Win32-x86 FLTK\Headers\FL" rd /S /Q "%CWFOLDER%\Win32-x86 FLTK\Headers\FL"
if exist "%CWFOLDER%\Win32-x86 FLTK\Libraries" rd /S /Q "%CWFOLDER%\Win32-x86 FLTK\Libraries"
if exist "%CWFOLDER%\Win32-x86 FLTK" rd /S /Q "%CWFOLDER%\Win32-x86 FLTK"

echo Deleting Fluid Plugin
if exist "%CWFOLDER%\Bin\Plugins\Compiler\fluid.exe" del "%CWFOLDER%\Bin\Plugins\Compiler\fluid.exe"
if exist "%CWFOLDER%\Bin\Plugins\Compiler\FluidCompiler.dll" del "%CWFOLDER%\Bin\Plugins\Compiler\FluidCompiler.dll"

echo Deleting FLTK Stationary
if exist "%CWFOLDER%\Stationery\Win32 C++\WIN32 FLTK GUI App" rd /S /Q "%CWFOLDER%\Stationery\Win32 C++\WIN32 FLTK GUI App"

echo .
pause

