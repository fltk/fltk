
Using Watcom to build and use FLTK 1.1.5

Supported targets: Win32 only, static builds (no DLLs). Can be used from any Watcom
   supported host (DOS, OS/2, Windows).

1. To build fltk itself:
   - set the environment variable fltk to the root directory of fltk
   - go to the Watcom directory, run wmake.
   Both debug and release versions of all libs, test programs and FLUID will be built.

2. To create you own programs using fluid:
   - set the environment variable fltk to the root directory of fltk
   - make a directory where you want to create the source of your program.
   - from fltk's watcom directory, copy file "makefile.example" to "makefile" in your
     directory.
   - Start fluid, and create your program.
   - You can either put in fluid's menu shell->execute command:
     "cmd /k "wmake && hello && exit || pause && exit"
     to compile and run the program (if you are running under Windows only),
   - Or in Fluid do shift-Ctrl-C to create C+= code, and then in a cmd prompt
     in your directory enter 'wmake", and then run the program.
   I have been able to do most of the Fluid Flash tutorial (no time to do all,
   but no error at all for the ones I did do.

3. Known errors:
    - Fluid aborts when exiting.

    Probably a lot more :-( . I cannot test/debug under NT or later, so I must
    rely on the help of others to find and fix :-) Watcom-related bugs.


Questions about the OpenWatcom port please to the fltk.general newsgroup.
Questions related to Openwatcom itself please to the c/c++ users group at
new.openwatcom.org.


Mat Nieuwenhoven, Hilversum, 2004-11-22








