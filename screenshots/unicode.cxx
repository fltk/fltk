//
// Unicode test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2025-2026 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//

// Generate screenshot with international text

#include <FL/Fl.H>
#include <FL/platform.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <string>

static const char *label_text =
  "ISO-8859-15: ¡¢£¤¥¦§¨©ª«¬­®¯ äöüß € µ ÀÁÂÃÄÅÆÇÈÉÊË ðñòóôõö÷øùúû\n"
  "Japanese:    FLTKは素晴らしいグラフィックライブラリです\n"
  "Chinese:     FLTK 是一個非常棒的圖形庫\n"
  "Greek:       Το FLTK είναι μια καταπληκτική βιβλιοθήκη γραφικών\n"
  "Korean:      FLTK는 훌륭한 그래픽 라이브러리입니다.\n"
  "Russian:     FLTK — это потрясающая графическая библиотека.\n"
  "Hindi:       FLTK एक शानदार ग्राफ़िक्स लाइब्रेरी है\n"
  "Armenian:    FLTK-ն հիանալի գրաֆիկական գրադարան է\n"
  "Arab:        FLTK هي مكتبة رائعة لواجهات المستخدم الرسومية\n"
  "Hebrew:      FLTK היא ספריית ממשק משתמש גרפי מעולה";

int main(int argc, char **argv) {
  static const Fl_Font font = FL_COURIER;
  static const int fsize = 20;
  static const Fl_Color bg = Fl_Color(0xf7f7ff00);
  fl_open_display();
  fl_font(font, fsize);
  int bw = 0, bh = 0;
  fl_measure(label_text, bw, bh, 0); // measure text
  auto win = new Fl_Double_Window(bw + 12, bh + 8, "FLTK international text");
  auto box = new Fl_Box(4, 4, bw, bh, label_text);
  box->box(FL_FLAT_BOX);
  box->color(bg);
  box->labelfont(font);
  box->labelsize(fsize);
  box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
  win->end();
  win->color(bg);
  win->resizable(box);
  win->size_range(win->w(), win->h());
  win->show(argc, argv);
  return Fl::run();
}
