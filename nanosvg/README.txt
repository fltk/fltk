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

$ git show --no-patch fltk_2021-09-13
tag fltk_2021-09-13
Tagger: Albrecht Schlosser <albrechts.fltk@online.de>
Date:   Mon Sep 13 19:09:40 2021 +0200

FLTK modifications as of Sep 13, 2021:

$ git shortlog master..fltk
AlbrechtS (2):
      Fix Visual Studio compilation error (missing long long).
      Modify rasterizer to support non-square X,Y axes scaling.

Greg Ercolano (1):
      Clip integer RGB percent values > 100

Latest upstream commit (master):

commit ccdb1995134d340a93fb20e3a3d323ccb3838dd0
Merge: 3cdd4a9 419782d
Author: Mikko Mononen <memononen@gmail.com>
Date:   Fri Sep 3 21:24:42 2021 +0300

    Merge pull request #198 from ctrlcctrlv/CVE_2019_1000032

    Fix decimal values in color fields (nsvg__parseColorRGB, nsvg__parseColorHex)

commit 461ad7de70d5fd3f09fc214e4baaadb830a2a270 (HEAD -> fltk, tag: fltk_2021-09-13, origin/fltk, origin/HEAD)
Author: Greg Ercolano <erco@seriss.com>
Date:   Mon Jan 18 15:05:13 2021 -0800

    Clip integer RGB percent values > 100
