@echo off

echo FLTK Fluid plugin installation for Metrowerks' CodeWarrior
echo .

:: --- copy Fluid and Fluid Plugin into the CW directories

if exist "%CWFOLDER%\Bin\Plugins\Compiler" goto copyPlugin

echo ERROR: Can't Fluid Plugin
echo Codewarrior Plugin folder not found
goto skipPluginCopy

:copyPlugin
echo Copying Fluid Plugin...
copy ..\..\fluid\fluid.exe "%CWFOLDER%\Bin\Plugins\Compiler"
copy ..\..\fluid\FluidCompiler.dll "%CWFOLDER%\Bin\Plugins\Compiler"

:skipPluginCopy

echo .
echo Metrowerks CodeWarrior needs to be restarted to make the
echo Fluid Plugin available. Please restart CodeWarrior now
echo to compile the FLTK Test and Demo files.
echo .
pause

