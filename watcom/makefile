#
# Name          : Watcom makefile for FLTK, multiplatform
#
# Author        : M. Nieuwenhoven
#
# The makefile has multiple targets:
# - clean       : Cleans all directories from files built during make
# - all         : Builds all targets for both debug and release
# - xy          : where x = D|R, Y = D|O|W|L, where only W (Win32) is
#                 currently supported by the source for Watcom.
#


!ifndef %FLTK
ERROR_MSG = Environment variable FLTK must have been set!
!endif

.BEFORE
!ifdef ERROR_MSG
    @echo .
    @echo $(ERROR_MSG)
    @echo .
    @%abort
!endif

ROOT=$(%FLTK)
PROJ=fltk

#
# This builds all library targets and the main programs
#
ALL : .SYMBOLIC
    set Clean=
    wmake -h dw
    wmake -h rw
#    wmake -h do
#    wmake -h ro
#    wmake -h dl
#    wmake -h rl

#
# Clean all directories. Call each target with a 'clean' target. Proper
# knowledge about cleaning can only be at the places where also the
# builds is done.
#
clean : .SYMBOLIC
    @echo Cleaning up.
    set Clean=CLEAN
    wmake -h dw
    wmake -h rw
#    wmake -h do
#    wmake -h ro
#    wmake -h dl
#    wmake -h rl

#
# Win32 Debug and Release targets
#
DW  : .SYMBOLIC
    @set OPTIONS=D=d O=w ROOT=$(ROOT) PROJ=$(PROJ) EXTRA_INCLUDE_DIRS=$(ROOT)
    %make BuildOne

RW  : .SYMBOLIC
    @set OPTIONS=D=r O=w ROOT=$(ROOT) PROJ=$(PROJ) EXTRA_INCLUDE_DIRS=$(ROOT)
    %make BuildOne

#
# OS/2 Debug and Release targets. Builds only jpeg, zlib and png libs for now.
#
DO  : .SYMBOLIC
    @set OPTIONS=D=d O=o ROOT=$(ROOT) PROJ=$(PROJ) EXTRA_INCLUDE_DIRS=$(ROOT)
    %make BuildOne

RO  : .SYMBOLIC
    @set OPTIONS=D=r O=o ROOT=$(ROOT) PROJ=$(PROJ) EXTRA_INCLUDE_DIRS=$(ROOT)
    %make BuildOne

#
# Linux Debug and Release targets. Environment variable XLIB must point to a directory
# which has a subdirectory X11 with X include files. Builds only jpeg, zlib and png libs for now.
#
DL  : .SYMBOLIC
    @set OPTIONS=D=d O=l ROOT=$(ROOT) PROJ=$(PROJ) EXTRA_INCLUDE_DIRS=$(ROOT);$(%XLIB)
    %make BuildOne

RL  : .SYMBOLIC
    @set OPTIONS=D=r O=l ROOT=$(ROOT) PROJ=$(PROJ) EXTRA_INCLUDE_DIRS=$(ROOT);$(%XLIB)
    %make BuildOne

# Internal target to build the individual modules. The following macros must
# have been set: D (value D or R), O (D,O,W,L)

BuildOne : .SYMBOLIC

    @echo "=== making jpeg library ==="
    @cd $(ROOT)\jpeg
    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
    @echo "=== making zlib library ==="
    @cd $(ROOT)\zlib
    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
    @echo "=== making png library ==="
    @cd $(ROOT)\png
    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
    @echo "=== making fltk libraries ==="
    @cd $(ROOT)\src
    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
    @echo "=== making fluid ==="
    @cd $(ROOT)\fluid
    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)
    @echo "=== making test ==="
    @cd $(ROOT)\test
    @wmake -h -f makefile.wat $(%OPTIONS) $(%CLEAN)


