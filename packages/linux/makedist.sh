#!/bin/sh
#
# makedist - make a linux distribution.
#
# Note: YOU MUST "MAKE INSTALL" FLTK PRIOR TO RUNNING THIS SCRIPT.
#       This is because the developers of the RPM distribution
#       format can't seem to realize that it would be nice to
#       make a distribution *without* first installing it, which
#       basically means you need to have two disks or systems in
#       order to test your distribution.  Ya, I think RPM is just
#       *great*...
#
#       Also, this script currently only builds a binary distribution.
#       FLTK's source tar file builds under Linux without any modification.
#

rpm -bb fltk.spec
