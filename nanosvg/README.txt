README for the nanosvg library bundled with FLTK
------------------------------------------------

This is a header-only library to display SVG images.

This bundled library was modified for optimal use in the FLTK library.


The original library can be found here:

  https://github.com/memononen/nanosvg


The modified library was forked and can be found here:

  https://github.com/fltk/nanosvg


For more information see README.bundled-libs.txt in FLTK's root directory.


Changes in the FLTK fork, branch 'fltk':
-----------------------------------------

$ git show --no-patch fltk_2022-12-22
tag fltk_2022-12-22
Tagger: Albrecht Schlosser <albrechts.fltk@online.de>
Date:   Thu Dec 22 00:44:33 2022 +0100

Latest nanosvg changes as of Dec 22, 2022:

$ git log -3 fltk
commit abcd277ea45e9098bed752cf9c6875b533c0892f (HEAD -> fltk)
Author: AlbrechtS <AlbrechtS.svn@fltk.example.org>
Date:   Sun Feb 4 23:47:38 2018 +0100

    Modify rasterizer to support non-square X,Y axes scaling.

    Add new function nsvgRasterizeXY() similar to nsvgRasterize() but with
    separate scaling factors for x-axis and y-axis.

commit 6ea21790604e7f0e84b2260fed34173ed475365e
Author: AlbrechtS <AlbrechtS.svn@fltk.example.org>
Date:   Sun Feb 4 23:43:30 2018 +0100

    Fix Visual Studio compilation error (missing long long).

    Change 'long long intPart' to 'double intPart' and replace
    strtoll() with _strtoi64() when built with Visual Studio.

commit 9da543e8329fdd81b64eb48742d8ccb09377aed1 (upstream/master)
Merge: c886e50 0ce2e2b
Author: Mikko Mononen <memononen@gmail.com>
Date:   Sun Dec 4 17:46:22 2022 +0200

    Merge pull request #236 from sezero/signed-char

    change struct NSVGpaint:type to signed char

----

Commits abcd277ea4 and 6ea2179060 are FLTK specific,
commit  9da543e832 is the latest upstream commit

---- End of tag ----

commit abcd277ea45e9098bed752cf9c6875b533c0892f (HEAD -> fltk, tag: fltk_2022-12-22)
Author: AlbrechtS <AlbrechtS.svn@fltk.example.org>
Date:   Sun Feb 4 23:47:38 2018 +0100

    Modify rasterizer to support non-square X,Y axes scaling.

    Add new function nsvgRasterizeXY() similar to nsvgRasterize() but with
    separate scaling factors for x-axis and y-axis.
