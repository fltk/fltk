
FLTK Project files for Metrowerks CodeWarrior 8 on Win32
--------------------------------------------------------

Loading:

  The Win32 version of the CW project file for Windows98,
  Windows2000, etc. is located in 
  fltk-1.1.x/CodeWarrior/Win32/FLTK.mcp
  
Targets:

  The first three targets can be use to install and test 
  FLTK for CodeWarrior. 
  
  The first target, 'Install FLTK Release', will compile 
  all FLTK libraries and install them in the Windows support
  folders of CW.
  
  'Install Fluid plugin' compiles the interactive UI builder
  Fluid and installs it as a compiler plugin in CW. Fluid
  resource files end with the extension ".fl" and can be added
  to a target using the 'File Mappings' panel. After adding 
  the Fluid plugin via 'run', CodeWarrior must be restarted.
  
  The third target, 'Build And Run Demos', will only compile
  without errors after the Fluid plugin has been installed.
  
Fluid Plugin:

  The Fluid Plugin allows .fl files to be compiled into .cxx
  and .h files. It will try to find Fluid (fluid.exe or
  fluidd.exe) first in one of the target paths, then in either
  CodeWarriors Bin and Copiler Plugin folder.

  The Plugin makes the home directory of the .fl file the 
  current directory and call Fluid with the -c option.

  After Fluid has created the .cxx and .h file, the plugin
  will notify CW that those files were externally changed.
  

Have fun!
  