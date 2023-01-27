Removed Experimental Platforms
------------------------------

Removed platforms and drivers include:

- Android
- Pico
- PicoAndroid
- PicoSDL

These platforms and their drivers were experimental and undocumented.

They have been removed in January 2022 for maintenance reasons as
discussed in fltk.coredev: "FLTK 1.4.0 release schedule", see:
https://groups.google.com/g/fltkcoredev/c/PDbHTRpXVh0/m/JqboexZ_AwAJ

More information, for instance where to find further development
(if any) will be added in this file.

If you want to take a look at the removed drivers you can checkout
the git tag "experimental-2022-01":

$ git clone https://github.com/fltk/fltk.git fltk-experimental
$ cd fltk-experimental
$ git checkout experimental-2022-01

You can also browse the files on GitHub:

https://github.com/fltk/fltk/tree/b275ff07158e80d1744ddb2f6c51094a87cf079a
