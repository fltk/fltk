#!/bin/sh
#
# makedist - make a linux distribution.
#
# Note: YOU MUST "MAKE INSTALL" FLTK PRIOR TO RUNNING THIS SCRIPT.
#
#       This is because the developers of the RPM distribution
#       tools don't have an easy way to install a set of files
#       to any location you want, e.g.:
#
#           destination-file = source-file
#
#       If you look at the other (commercial) UNIX distributions
#       you'll notice that ALL of them (except for Digital UNIX)
#       support this syntax in one form or another.
#
#       Several folks have pointed out the "build root" stuff
#       provided by RPM.  There are two problems with this:
#
#           1. You still need to install the files somewhere prior
#              to making the installation.
#           2. Users can then install the software at a different
#              location, which will cause a lot of problems with
#              the FLTK DSOs.
#
#       Also, this script currently only builds a binary distribution.
#       FLTK's source tar file builds under Linux without any modification.
#
#       Finally, if you ask me the RPM install process is simple and
#       slick.  You can rebuild software from source automatically, etc.
#       If it weren't for the fact that the RPM build process is so
#       bass ackwards I'd love it completely.
#

rpm -bb fltk.spec
