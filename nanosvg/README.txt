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

$ git show --no-patch fltk_2021-02-22
tag fltk_2021-02-22
Tagger: Albrecht Schlosser <...>
Date:   Mon Feb 22 14:16:58 2021 +0100

Included in FLTK 1.4.x as of Feb 22, 2021

Latest upstream changes:
------------------------

commit 3e403ec72a9145cbbcc6c63d94a4caf079aafec2
Merge: cc6c08d 45eb9f8
Author: Mikko Mononen <...>
Date:   Fri Nov 20 12:53:11 2020 +0200

    Merge pull request #189 from fvogelnew1/Fix-for-#188

    Update nanosvg.h

Changes in branch 'fltk':

  $ git shortlog master..fltk

    AlbrechtS (2):
      Fix Visual Studio compilation error (missing long long).
      Modify rasterizer to support non-square X,Y axes scaling.

    Greg Ercolano (1):
      Address crash defined in fltk's issue 180

commit a1eea27b3db2d15d924ea823dd0acc5bd2aa56f1
Author: Greg Ercolano <...>
Date:   Mon Jan 18 15:05:13 2021 -0800

    Address crash defined in fltk's issue 180
