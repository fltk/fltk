// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

Fl_Terminal Design Document
===========================

When I started this project, I identified the key concepts needed to
implement Fl_Terminal:

        - Draw and manage multiline Unicode text in FLTK effectively,
          allowing per-character colors and attributes like underline,
          strikeout, background, etc.

        - An efficient screen buffer to handle the "scrollback history"
          and "screen display" concepts; the "history" being a scrollback
          history of text that scrolls up and off screen from the "display",
          and the "display" being where the action is: the cursor can be
          moved around and text scrolled up or down.

        - How the vertical scrollbar should provide the user with a way to
          scroll back into the scrollback history to allow the user to scroll back
          to view the "scrollback history", without stopping the "screen display"
          from operating

        - How to manage mouse selection for copy/paste

        - Escape code management to implement VT100 style / ANSI escape codes.


     ┌─────────────────────────────────────────┬──────────────────────────────┐
     │  NOTE: Abbreviations "hist" and "disp"  │                              │
     ├─────────────────────────────────────────┘                              │
     │                                                                        │
     │  "history" may be abbreviated as "hist", and "display" as "disp" in    │
     │  both this text and the source code. 4 character names are used so     │
     │  they line up cleanly in the source, e.g.                              │
     │                                                                        │
     │      ring_cols = 0;  ring_rows = 0;                                    │
     │      hist_cols = 0;  ring_cols = 0;                                    │
     │      disp_cols = 0;  ring_cols = 0;                                    │
     │      └─┬┘ └─┬┘                                                         │
     │        └────┴─── 4 characters                                          │
     │                                                                        │
     └────────────────────────────────────────────────────────────────────────┘

So the  of these concepts were able to fit into C++ class concepts well.
Those classes being:

        Utf8Char
        ========
        Each character on the screen is a "Utf8Char" which can manage
        the utf8 encoding of any character as one or more bytes. Also
        in that class is a byte for an attribute (underline, bold, etc),
        and two integers for fg/bg color.

        RingBuffer
        ==========
        The RingBuffer class keeps track of the buffer itself, a single
        array of Utf8Chars called "ring_chars", and some index numbers
        to keep track of how many rows are in the screen's history and
        display, named "hist_rows" and "disp_rows".

        The memory layout of the Utf8Char array is:

                      ___________________   _ _
                     |                   |   ʌ
                     |                   |   |
                     |                   |   |
                     |   H i s t o r y   |   |  hist_rows
                     |                   |   |
                     |                   |   |
                     |___________________|  _v_
                     |                   |   ʌ
                     |                   |   |
                     |   D i s p l a y   |   |  disp_rows
                     |                   |   |
                     |___________________|  _v_

                     |<----------------->|
                           ring_cols

        So it's basically a single continguous array of Utf8Char instances
        where any character can be accessed by index# using the formula:

                ring_chars[ (row*ring_cols)+col ]

        ..where 'row' is the desired row, 'col' is the desired column,
        and 'ring_cols' is how many columns "wide" the buffer is.

        Methods are used to give access the characters in the buffer.

        A key concept is to allow the starting point of the history and
        display to be moved around to implement 'text scrolling', such
        as when crlf at the screen bottom causes a 'scroll up'.

        This is simply an "index offset" integer applied to the
        hist and disp indexes when drawing the display, e.g.

              Offset is 0:           2    Offset now 2:
             ┌───────────────────┐ ──┐   ┌───────────────────┐
             │                   │   │   │   D i s p l a y   │
             │                   │   └─> ├───────────────────┤
             │                   │       │                   │
             │   H i s t o r y   │       │                   │
             │                   │       │   H i s t o r y   │
             │                   │   2   │                   │
             ├───────────────────┤ ──┐   │                   │
             │                   │   │   │                   │
             │                   │   └─> ├───────────────────┤
             │   D i s p l a y   │       │                   │
             │                   │       │    D i s p l a y  │
             │                   │       │                   │
             └───────────────────┘       └───────────────────┘

              Offset is 0:           4    Offset now 4:
             ┌───────────────────┐ ──┐   ┌───────────────────┐
             │                   │   │   │                   │
             │                   │   │   │   D i s p l a y   │
             │                   │   │   │                   │
             │   H i s t o r y   │   └─> ├───────────────────┤
             │                   │       │                   │
             │                   │   4   │                   │
             ├───────────────────┤ ──┐   │   H i s t o r y   │
             │                   │   │   │                   │
             │                   │   │   │                   │
             │   D i s p l a y   │   │   │                   │
             │                   │   └─> ├───────────────────┤
             │                   │       │   D i s p l a y   │
             └───────────────────┘       └───────────────────┘

        The effect of applying an offset trivially implements "text scrolling",
        so that no screen memory has to physically moved around, simply changing
        the single integer "offset" is enough. The text remains where it was, and
        the offset is simply incremented to scroll up. This also automatically
        makes it appear the top line in the display is 'scrolled up' into the
        last line of the scrollback history.

        If the offset exceeds the size of the ring buffer, it is simply wrapped
        back to the beginning of the buffer with a modulo: offset =% ring_rows;

        Indexes into the display and history are also modulo their respective
        rows, e.g.

            act_ring_index = (hist_rows + disp_row + offset - scrollbar_pos) % ring_rows;

        This way indexes for ranges can run beyond the bottom of the ring,
        and automatically wrap around the ring, e.g.

                             Offset now 4:
                            ┌───────────────────┐
                          2 │                   │
                          3 │   D i s p l a y   │
                          4 │                   │  <- act_disp_row(4)
                            ├───────────────────┤
                            │                   │
                            │                   │
                            │   H i s t o r y   │
                            │                   │
                            │                   │
                            │                   │
                            ├───────────────────┤
                          0 │   D i s p l a y   │
                          1 └───────────────────┘  <- ring_rows
                          2 :                   :
                          3 :                   :
           disp_row(5) -> 4 :...................:

       Here the "disp_row" is the desired offset into the display, but we
       need the actual index into the ring from the top, since that's the
       physical array.

       So some simple math calculates the row position based on the "offset",
       and the "hist" vs "disp" concepts:

           act_ring_index = (histrows          // the display exists AFTER the history, so offset the hist_rows
                             + offset          // include the scroll 'offset'
                             + disp_row        // add the desired row relative to the top of the display (0..disp_rows)
                            ) % ring_rows;     // make sure the resulting index is within the ring buffer (0..ring_rows)

       An additional bit of math makes sure if a negative result occurs, that
       negative value works relative to the end of the ring, e.g.

           if (act_ring_index < 0) act_ring_index = ring_rows + act_ring_index;

       This guaratnees the act_ring_index is within the ring buffer's address space,
       with all offsets applied.

       The math that implements this can be found in the u8c_xxxx_row() methods,
       where "xxxx" is one of the concept regions "ring", "hist" or "disp":

           Utf8Char *u8c;
           u8c = u8c_ring_row(rrow);    // address within ring, rrow can be 0..(ring_rows-1)
           u8c = u8c_hist_row(hrow);    // address within hist, hrow can be 0..(hist_rows-1)
           u8c = u8c_disp_row(drow);    // address within disp, drow can be 0..(disp_rows-1)

       The small bit of math is only involved whenever a new row address is needed,
       so in a display that's 80x25, to walk all the characters in the screen, the
       math above would only be called 25 times, once for each row, e.g.

             for ( int row=0; row<disp_rows(); row++ ) {    // walk rows: disp_rows = 25
               Utf8Char *u8c = u8c_disp_row(row);           // get first char in display 'row'
               for ( int col=0; col<disp_cols(); col++ ) {  // walk cols: disp_cols = 80
                 u8c[col].do_something();                   // work with the character at row/col
               }
             }

       So to recap, the concepts here are:

          - The ring buffer itself, a linear array that is conceptually
            split into a 2 dimensional array of rows and columns whose
            height and width are:

                ring_rows -- how many rows in the entire ring buffer
                ring_cols -- how many columns in the ring buffer
                nchars    -- total chars in ring, e.g. (ring_rows * ring_cols)

          - The "history" within the ring. For simplicity this is thought of
            as starting relative to the top of the ring buffer, occupying
            ring buffer rows:

                0..(hist_rows-1)

          - The "display", or "disp", within the ring, just after the "history".
            It occupies the ring buffer rows:

                (hist_rows)..(hist_rows+disp_rows-1)

            ..or similarly:

                (hist_rows)..(ring_rows-1)

          - An "offset" used to move the "history" and "display" around within
            the ring buffer to implement the "text scrolling" concept. The offset
            is applied when new characters are added to the buffer, and during
            drawing to find where the display actually is within the ring.

          - A "scrollbar", which only is used when redrawing the screen the user sees,
            and is simply an additional offset to all the above, where a scrollback
            value of zero (the scrollbar tab at the bottom) shows the display rows,
            and the values increase as the user moves the scrolltab upwards, 1 per line,
            which is subtracted from the normal starting index to let the user work their
            way backwards into the scrollback history.

        The ring buffer allows new content to simply be appended to the ring buffer,
        and the index# for the start of the display and start of scrollback history are
        simply incremented. So the next time the display is "drawn", it starts at
        a different position in the ring.

        This makes scrolling content at high speed trivial, without memory moves.
        It also makes the concept of "scrolling" with the scrollbar simple as well,
        simply being an extra index offset applied during drawing.

        If the display is enlarged vertically, that's easy too; the display
        area is simply defined as being more rows, the history as less rows,
        the history use decreased (since what was in the history before is now
        being moved into the display), and all the math adjusts accordingly.

Mouse Selection
===============

Dragging the mouse across the screen should highlight the text, allowing the user
to extend the selection either beyond or before the point started. Extending the
drag to the top of the screen should automatically 'scroll up' to select more
lines in the scrollback history, or below the bottom to do the opposite.

The mouse selection is implemented as a class to keep track of the start/end
row/col positions of the selection, and other details such as a flag indicating
if a selection has been made, what color the fg/bg text should appear when
text is selected, and methods that allow setting and extending the selection,
clearing the selection, and "scrolling" the selection, to ensure the row/col
indexes adjust correctly to track when the screen or scrollbar is scrolled.


Redraw Timer
============

Knowing when to redraw is tricky with a terminal, because sometimes high volumes
of input will come in asynchronously, so in that case we need to determine when
to redraw the screen to show the new content; too quickly will cause the screen
to spend more time redrawing itself, preventing new input from being added. Too
slowly, the user won't see new information appear in a timely manner.

To solve this, a rate timer is used to prevent too many redraws:

    - When new data comes in, a 1/10 sec timer is started and a modify flag is set.

      redraw() is NOT called at this time, allowing new data to continue to arrive
      quickly. Once the modify flag is set, nothing changes from there.

    - When the 1/10th second timer fires, the callback checks the modify flag:

            - if set, calls redraw(), resets the modify to 0, and calls
              Fl::repeat_timeout() to repeat the callback in another 1/10th sec.

            - if clear, no new data came in, so DISABLE the timer, done.

In this way, redraws don't happen more than 10x per second, and redraw() is called
only when there's new content to see.

The redraw rate can be set by the user application using the Fl_Terminal::redraw_rate(),
0.10 being the default.

Some terminal operations necessarily call redraw() directly, such as interactive mouse
selection, or during user scrolling the terminal's scrollbar, where it's important there's
no delay in what the user sees while interacting directly with the widget.

TERMINAL RESIZING
=================
Resizing currently follows xterm(1) style behavior:

    CASE 1: ENLARGE, USING HISTORY:
           -2  Line 1   ┐_ scrollback
           -1  Line 2   ┘  history
             ┌──────────────────┐              ┌──────────────────┐
            1│ Hello!           │             1│ Line 1           │
            2│ ▒                │   enlarge   2│ Line 2           │
             └──────────────────┘ ──┐         3│ Hello!           │
                                    │         4│ ▒                │
                                    └──────>   └──────────────────┘
    ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄
    CASE 2: ENLARGE, ADDING LINES BELOW CURSOR
             ┌──────────────────┐              ┌──────────────────┐
            1│ Line 1           │             1│ Line 1           │
            2│ Line 2           │   enlarge   2│ Line 2           │
            3│ Hello!           │             3│ Hello!           │
            4│ ▒                │             4│ ▒                │
             └──────────────────┘ ──┐         5│                  │
                                    │         6│                  │
                                    └──────>   └──────────────────┘
    ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄
    CASE 3: SHRINK, TRIMMING LINES BELOW CURSOR
             ┌──────────────────┐            ┌──────────────────┐
            1│ Line 1           │           1│ Line 1           │
            2│ Line 2           │           2│ Line 2           │
            3│ Hello!           │   resize  3│ Hello!           │
            4│ ▒                │           4│ ▒                │
            5│                  │   ╭────>   └──────────────────┘
            6│                  │   │       5    ┐ Lines below cursor erased
             └──────────────────┘ ──╯       6    ┘ (regardless of contents)
    ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄ ┄
    CASE 4: SHRINK, PUSH TO HISTORY:       -2  Line 1   ┐_ moved into
                                           -1  Line 2   ┘  scrollback history
             ┌──────────────────┐            ┌──────────────────┐
            1│ Line 1           │           1│ Hello!           │
            2│ Line 2           │   resize  2│ ▒                │
            3│ Hello!           │   ╭────>   └──────────────────┘
            4│ ▒                │   │
             └──────────────────┘ ──╯

These case numbers (CASE 1 tru 4) are referenced in the source code for
Fl_Terminal::refit_disp_to_screen(void).


OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD
 OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD
  OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD
   OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD     OLD



RING BUFFER DESCRIPTION
=======================

  The history and display are defined by row indexes into this buffer
  which are adjusted whenever the text display is 'scrolled' (e.g. by crlfs)

  The scrollbar is a secondary offset on top of that applied during drawing.
  the display.

  Here's what the variables managing the split ring buffer look like:



                                          RING BUFFER
                  ─   ring_chars[] ─┬─>┌───────────────┐
                  ʌ   [hist_srow] ──┘  │               │
                  ┊                    │               │
                  ┊                    │ H i s t o r y ┊
        hist_rows_┊                    │               │
                  ┊                    │               │
                  ┊                    │               │
                  ┊                    │               │
                  v    [hist_erow] ───>│               │
                  ─    [disp_srow] ───>├───────────────┤
                  ʌ                    │               │
                  ┊                    │               │
        disp_rows_┊                    │ D i s p l a y │
                  ┊                    │               │
                  ┊                    │               │
                  v    [ring_erow] ───>│              +│<── ring_bot_
                  ─                    └───────────────┘    (last valid ptr address in ring)

                                       │<──────┬──────>│
                                           ring_cols
                                           hist_cols
                                           disp_cols

  The concept here is the single ring buffer is split into two parts that can differ
  in height (rows) but not in width (cols). For instance, typically the history is
  many times larger than the display; a typical old school display might be 80x25,
  but the history might be 2000 lines.

  ring_row() handles the fact that 'row' might run off the end of the buffer,
  depending on where hist_srow starts. For instance, if the display has scrolled
  a few lines, the ring buffer arrangement might look like:

                                          RING BUFFER
                      ring_chars[] ───>┌───────────────┐
                                       │ D i s p l a y │
                       [disp_erow] ───>│               │
                                       ├───────────────┤
                       [hist_srow] ───>│               │
                                       │               │
                                       │ H i s t o r y │
                                       │               │
                                       │               │
                                       │               │
                       [hist_erow] ───>│               │
                                       ├───────────────┤
                       [disp_srow] ───>│               │
                                       │ D i s p l a y │
                                       │               │
                       [ring_erow] ───>│              +│<── ring_bot_
                                       └───────────────┘    (last valid ptr address in ring)


  Note how the display 'wraps around', straddling the end of the ring buffer.
  So trivially walking ring_chars[] from 'disp_srow' for 'disp_rows' would run
  off the end of memory for the ring buffer. Example:

     // BAD!
     for ( int row=disp_srow; row<disp_rows; row++ ) {
       for ( int col=0; col<disp_cols; col++ ) {
         ring_chars[row*disp_cols+col]->do_something();  // BAD! can run off end of array
       }
     }

  The function u8c_row() can access the Utf8Char* of each row more safely,
  ensuring that even if the 'row' index runs off the end of the array,
  u8c_row() handles wrapping it for you. So the safe way to walk the chars
  of the display can be done this way:

     // GOOD!
     for ( int row=disp_srow; row<disp_rows; row++ ) {
       Utf8Char *u8c = u8c_row(row);              // safe: returns utf8 char for start of each row
       for ( int col=0; col<disp_cols; col++ ) {  // walk the columns safely
         (u8c++)->do_something();                 // get/set the utf8 char
       }
     }

  Walking the history would be the same, just replace disp_xxxx with hist_xxxx.

  One can also use ring_row_normalize() to return an index# that can be directly
  used with ring_chars[], the value kept in range of the buffer.


RING BUFFER "SCROLLING"
=======================
   A ring buffer is used to greatly simplify the act of 'scrolling', which happens a lot
   when large amounts of data come in, each CRLF triggering a "scroll" that moves the top
   line up into the history buffer. The history buffer can be quite large (1000's of lines),
   so it would suck if, on each line scroll, thousands of rows of Utf8Chars had to be
   physically moved in memory.

   Much easier to just adjust the srow/erow pointers, which simply affect how drawing
   is done, and where the display area is. This trivially handles scrolling by just
   adjusting some integers by 1.


                          --   --  HOW SCROLLING UP ONE LINE IS DONE  --   --


          ring_chars[] ─┬─>┌───────────────┐ ─┐   ring_chars[] ──>┌─────────────────┐
          [hist_srow] ──┘  │               │  │                   │x x x x x x x x x│ <-- blanks
                           │               │  └─>  [hist_srow] ──>├─────────────────┤
                           │    H i s t    │                      │                 │
                           │               │                      │                 │
                           │               │                      │     H i s t     │
                           │               │                      │                 │
                           │               │                      │                 │
           [hist_erow] ───>│               │                      │                 │
           [disp_srow] ───>├Line 1─────────┤ ─┐    [hist_erow] ──>│Line 1           │
                           │Line 2         │  └─>  [disp_srow] ──>├Line 2───────────┤
                           │Line 3         │                      │Line 3           │
                           │               │                      │                 │
                           │    D i s p    │                      │     D i s p     │
                           │               │                      │                 │
[disp_erow][ring_erow] ───>│Line 24       +│       [ring_erow] ──>│Line 24         +│
                           └───────────────┘                      └─────────────────┘

  In the above, Line 1 has effectively "moved" into history because the disp_s/erow
  and hist_s/erow variables have just been incremented.

  During resize_display(), we need to preserve the display and history as much as possible
  when the ring buffer is enlarged/shrank; the hist_rows size should be maintained, and only
  display section changes size based on the FLTK window size.



===================== OLD ====================== OLD ====================== OLD ======================

Conventions used for the internals
==================================
This is a large widget, and these are some breadcrumbs for anyone
working on the internals of this class.

    > There is one utf8 char buffer, buff_chars[], the top part is the 'history buffer'
      (which the user can scroll back to see), and the 'display buffer' which is the
      'active display'.
    > glob or global  - refers to global  buffer buff_chars[]
    > disp or display - refers to display buffer disp_chars[]
    > Abbreviations glob/disp/buff/hist used because 4 chars line up nicely
    > row/col variable names use a 'g' or 'd' prefix to convey 'g'lobal or 'd'isplay.
    > The 'Cursor' class uses row/col for the display (disp_chars[]) because the
      cursor is only ever inside the display.
    > The 'Selection' class uses row/col for the global buffer (buff_chars[])
      because it can be in the 'history' or the 'display'
    > These concepts talk about the same thing:
          > global buffer == buff_chars[] == "history buffer" == hist == grow/gcol
          > display == disp_chars[] == "active display" == drow/dcol
    > There is no hist_chars[] because it's just the top half of buff_chars[]
    > There is no hist_height_ because it's the same as hist_max_
    > There is no hist_width_ because it's the same as buff_width_.


Fl_Terminal's Class Hierarchy
=============================

 class Fl_Terminal     -- Derived from Fl_Group (to parent scrollbars, popup menus, etc)
                          We mainly use the group's background to draw over in draw().
                          Within the terminal classes are the following private/protected classes
                          that help with bookkeeping and operation of the terminal class:

   class Margin        -- Handles the margins around the terminal drawing area
   class CharStyle     -- The styling for the characters: single byte color + attribute (bold/inverse/etc)
   class Cursor        -- The attributes of the cursor -- position, color, etc, and some simple movement logic
   class Utf8Char      -- Visible screen buffer is an array of these, one per character
   class RingBuffer    -- The ring buffer of Utf8Char's, with the "history" and "display" concept.
   class EscapeSeq     -- A class to handle parsing Esc sequences, and keeping state info between chars
                          Single chars go in, and when a complete esc sequence is parsed, the caller
                          can find out all the integer values and command code easily to figure out
                          what op to do.

 OVERALL DESIGN:
   To handle unicode, the terminal's visible display area is a linear array of pointers to
   instances of the 'Utf8Char' class, one instance per character. The arrangement of the array
   is much like the IBM PC's video memory, but instead of Char/Attrib byte pairs, the Utf8Char
   class handles the more complex per-character data and colors/attributes.

   The cursor x,y value can be quickly converted to an index into this buffer.
   Strings are printed into the buffer, again, similar to the IBM PC video memory;
   one character at a time into the Utf8Char class instances.

   When the screen redraws, it just walks this array, and calls fl_draw() to draw
   the text, one utf8 char at a time, with the colors/fonts/attributes from the Utf8Char class.

   As characters are added, Esc sequences are intercepted and parsed into the EscapeSeq class,
   which has a single instance for the terminal.

   For the scrollback history, as lines scrolls off the top of the active display area,
   the Utf8Char's are copied to the history buffer, and the active display's top line
   is simply rotated to the bottom line and cleared, allowing memory reuse of the Utf8Char's,
   to prevent memory churn for the display. The goal is to allow high volume output to the
   terminal with a minimum affect on realloc'ing memory.

 OPTIMIZATIONS
   Where possible, caching is used to prevent repeated calls to cpu expensive operations,
   such as anything to do with calculating unicode character width/height/etc.

RingBuffer
   The ring buffer is split in two; the top part is the history, the bottom part is
   the "display area", where new text comes in, where the cursor can be positioned,
   and concepts like "scroll up" and "scroll down" all happen. The "history" is simply
   a linear buffer where lines pushed up from the display are moved into.

   Methods let one access the ring with index#s:

    - The entire ring buffer can be accessed with:

          for (int i=0; i<ring.ring_rows(); i++) {
            Utf8Char *u8c_row = ring.u8c_ring_row(i);
            for (int col=0; i<ring.ring_cols(); i++) {
              u8c_row[col].xxx();               // access each Utf8Char at the row/col
            }
          }

                          Row#s can be given that are larger than the ring; these are automatically
                          wrapped around to the top. The ring, "history" and "display" can each be
                          accessed separately with index#s relative to their position
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
    Moved this down to the bottom of the file for now -- not sure where to put this,
    but it's useful if one wants to reuse the EscapeSeq class somewhere else. -erco Dec 2022

    Typical use pattern of EscapeSeq class.
    This is unverified code, but should give the general gist;

       while ( *s ) {                                // walk text that may contain ESC sequences
         if ( *s == 0x1b ) {
           escseq.parse(*s++);                       // start parsing ESC seq (does a reset())
           continue;
         } else if ( escseq.parse_in_progress() ) {  // continuing to parse an ESC seq?
           switch (escseq.parse(*s++)) {             // parse char, advance s..
             case fail:    escseq.reset(); continue; // failed? reset, continue..
             case success: continue;                 // keep parsing..
             case completed:                         // parsed complete esc sequence?
               break;
           }
           // Handle parsed esc sequence here..
           switch ( escseq.esc_mode() ) {
             case 'm':                               // ESC[...m?
               for ( int i=0; i<escseq.total_vals(); i++ ) {
                 int val = escseq.val(i);
                 ..handle values here..
               }
               break;
             case 'J':                               // ESC[#J?
               ..handle..
               break;
           }
           escseq.reset();   // done handling escseq, reset()
           continue;
         } else {
           ..handle non-escape chars here..
         }
         ++s;    // advance thru string
       }

----------------------------------------------------------------------------------------

