#! /bin/sh
#
# This file is no longer referred to in the documentation, it's kept
# for backwards compatibility only.
#
# Just run `autoconf` instead to generate `configure` and you're done.
#
# The old README.Unix.txt stated that it should be executed from within
# the "FLTK source-code directory", hence changing directories is not
# useful and would break if the user's home directory contained spaces.
# Changing directories has been removed in FLTK 1.4.0 and this file
# has been simplified substantially.
#
# Instead of executing it as documented in pre-1.4 README files the new docs
# instruct to just execute `autoconf` which is sufficient and equivalent to
# the old instructions.

autoconf --force || exit 1

test -n "$NOCONFIGURE" || ./configure "$@"
