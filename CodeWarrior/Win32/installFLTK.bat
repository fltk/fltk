@echo off

echo FLTK library installation for Metrowerks' CodeWarrior
echo .

:: --- copy FLTK header files into Win32 support directory

if exist "%CWFOLDER%" goto copyHeaders

echo ERROR: Can't copy headers files
echo Codewarrior header files folder not found
goto skipHeaderCopy

:copyHeaders
echo Copying header files...
xcopy /Y /C /E /I /Q ..\..\FL "%CWFOLDER%\Win32-x86 FLTK\Headers\FL"

:skipHeaderCopy


:: --- copy FLTK libraries into Win32 support directory

if exist "%CWFOLDER%" goto copyLibs

echo ERROR: Can't copy libraries
echo Codewarrior libraries folder not found
goto skipLibsCopy

:copyLibs
echo Copying libraries...
xcopy /Y /C /E /I /Q ..\..\lib "%CWFOLDER%\Win32-x86 FLTK\Libraries"

:skipLibsCopy

:: --- copy FLTK dll into test directory to make 'editor' test work

echo Copying dll to 'test'...
xcopy /Y /Q ..\..\lib\fltkdll.dll ..\..\test

:: --- copy FLTK stationary into stationary directory

if exist "%CWFOLDER%\Stationery\Win32 C++" goto copyStat

echo ERROR: Can't copy stationary
echo Codewarrior stationary folder not found
goto skipStatCopy

:copyStat
echo Copying FLTK stationary...
xcopy /Y /C /E /I /Q "stationary" "%CWFOLDER%\Stationery\Win32 C++\WIN32 FLTK GUI App"

:skipStatCopy

echo .
pause

