//
// "$Id$"
//
// 	Simple example of a button with text over an image
// 	Originally from erco's cheat sheet 10/25/2010, permission by author.
//
//	This shows how to include an 'inline' image (.xpm)
//	and have it appear on an Fl_Button. Demonstrates the use of the
//	FL_ALIGN_IMAGE_BACKDROP align() flag (new in FLTK 1.3.0).
//
//	Note that the XPM can just as easily be in an #include file,
//	but to keep the example self contained, the image (a gray scale
//	gradient) is included here.
//
// Copyright 2010 Greg Ercolano.
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pixmap.H>

/* XPM */
static const char * gradient_xpm[] = {
"135 20 26 1",
"a 	c #e0e0e0", "b 	c #dcdcdc", "c 	c #d8d8d8", "d 	c #d4d4d4", "e 	c #d2d2d2",
"f 	c #d0d0d0", "g 	c #cccccc", "h 	c #c8c8c8", "i 	c #c4c4c4", "j 	c #c2c2c2",
"k 	c #c0c0c0", "l 	c #bcbcbc", "m 	c #b8b8b8", "n 	c #b4b4b4", "o 	c #b2b2b2",
"p 	c #b0b0b0", "q 	c #acacac", "r 	c #a8a8a8", "s 	c #a4a4a4", "t 	c #a2a2a2",
"u 	c #a0a0a0", "v 	c #9c9c9c", "w 	c #989898", "x 	c #949494", "y 	c #929292",
"z 	c #909090",
"aaaaaaaaabbbbbbbbcccccccddddddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrssss",
"aaaaaabbbbbbbbcccccccddddddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssst",
"aaabbbbbbbbcccccccddddddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssstt",
"bbbbbbbbcccccccddddddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrssssssssstttt",
"bbbbbcccccccddddddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttu",
"bbcccccccddddddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuu",
"ccccccddddddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuu",
"cccddddddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvv",
"ddddddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvv",
"dddeeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvw",
"eeeeeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwww",
"eeefffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwwwwww",
"fffffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwwwwwwwxx",
"ffffgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwwwwwwwxxxxx",
"fgggggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwwwwwwwxxxxxxyy",
"gggggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwwwwwwwxxxxxxyyyyy",
"ggghhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwwwwwwwxxxxxxyyyyyyyy",
"hhhhhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwwwwwwwxxxxxxyyyyyyyyzzz",
"hhhhiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwwwwwwwxxxxxxyyyyyyyyzzzzzz",
"hiiiiiiijjjjjjjkkkkkkkklllllllmmmmmmmnnnnnnnnnoooooooppppppppqqqqqqrrrrrrrsssssssssttttttuuuuuuuuvvvvvvvwwwwwwwwxxxxxxyyyyyyyyzzzzzzzzz"};

int main(int argc, char **argv) {
    Fl_Pixmap gradient(gradient_xpm);
    Fl_Window *win = new Fl_Window(160, 75, "test");          // create window

    Fl_Button *but1 = new Fl_Button(10,10,140,25,"Button 1"); // create regular button
    but1->image(&gradient);                                   // assign it an image
    but1->align(FL_ALIGN_IMAGE_BACKDROP|but1->align());       // use image as a 'backdrop'

    Fl_Button *but2 = new Fl_Button(10,40,140,25,"Button 2"); // create second instance of button
    but2->image(&gradient);                                   // assign it same image
    but2->align(FL_ALIGN_IMAGE_BACKDROP|but2->align());       // use image as a 'backdrop'

    win->end();
    win->show(argc,argv);
    return(Fl::run());
}

//
// End of "$Id$".
//
