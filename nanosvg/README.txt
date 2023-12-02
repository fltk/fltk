README for the nanosvg library bundled with FLTK
------------------------------------------------

This is a header-only library to display SVG images.

This bundled library was modified for optimal use in the FLTK library.


The original library can be found here:

  https://github.com/memononen/nanosvg


The modified library was forked and can be found here:

  https://github.com/fltk/nanosvg


For more information see documentation/src/bundled-libs.dox.


Changes in the FLTK fork, branch 'fltk':
-----------------------------------------

See current branch 'fltk' and tag 'fltk_2023-12-02' in FLTK's
nanosvg fork (link above).

FLTK specific commits can be found with git similar to the following
command. Note that Git hashes will change over time whenever the
FLTK specific changes are rebased on top of 'upstream/master'.


$ git log upstream/master..fltk
commit 7aeda550a84c15680f7e55867896c3906299dffb (HEAD -> fltk, tag: fltk_2023-12-02, origin/fltk, origin/HEAD)
Author: AlbrechtS <AlbrechtS.svn@fltk.example.org>
Date:   Sun Feb 4 23:47:38 2018 +0100

    Modify rasterizer to support non-square X,Y axes scaling.

    Add new function nsvgRasterizeXY() similar to nsvgRasterize() but with
    separate scaling factors for x-axis and y-axis.

commit b9a21ceb59590e0325acbf290598cbdf490501d6
Author: AlbrechtS <AlbrechtS.svn@fltk.example.org>
Date:   Sun Feb 4 23:43:30 2018 +0100

    Fix Visual Studio compilation error (missing long long).

    Change 'long long intPart' to 'double intPart' and replace
    strtoll() with _strtoi64() when built with Visual Studio.
