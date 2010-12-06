@echo off
echo Compiling fluid files ...
pushd .
cd ../../test
if "%1"=="/D" goto debugmode
..\fluid\fluid -c "%1"
goto end

:debugmode:
..\fluid\fluidd -c "%2"

:end
popd
