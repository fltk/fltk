//
// "$Id$"
//
// Doxygen pre-formatting program for the Fast Light Tool Kit (FLTK).
//
// Copyright 2010 by Matthias Melcher.
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

#include <stdio.h>
#include <string.h>

char linebuf[1024];



int main(int argc, char **argv) {
  if (argc!=1) {
    puts("Add stars (*) in front of multi-line doxygen comments");
    puts("to protect comment indentation from code beautifiers.");
    puts("usage: cat file | doxystar");
    return 0;
  }

  int state = 0;
  char *commentStart;
  int i, commentCol;
  for (;;) {
    if (!fgets(linebuf, 1020, stdin)) break; // EOF or error
    switch (state) {
      case 0: // line start is source code
        commentStart = strstr(linebuf, "/*"); 
        if (commentStart) {
          // check if this comment spans multiple lines
          if (strstr(commentStart, "*/")==0) {
            if ((commentStart[2]=='*' || commentStart[2]=='!') && commentStart[3]!='*') {
              state = 2; // Doxygen multiline comment
              commentCol = commentStart - linebuf;
            } else {
              state = 1; // regular multiline comment
            }
          } else { 
            // single line comment, do nothing
          }
        }
        fputs(linebuf, stdout);
        break;
      case 1: // line start is inside a regular multiline comment
        if (strstr(linebuf, "*/")) {
          state = 0;
        } else {
          // still inside comment
        }
        fputs(linebuf, stdout);
        break;
      case 2: // line start is inside a doxygen  multiline comment
        for (i=0; i<commentCol; i++) fputc(' ', stdout);
        fputs(" *", stdout); 
        if (strstr(linebuf, "*/")) {
          state = 0;
        } else {
          // still inside comment
        }
        for (i=0; i<commentCol+1; i++) 
          if (linebuf[i]!=' ')
            break;
        if (linebuf[i]=='*') {
          if (linebuf[i+1]==' ') {
            i+=2;
          } else {
            i+=1;
          }
        }
        fputs(linebuf+i, stdout);
        break;
    }
  }

  return 0;
}

//
// End of "$Id$".
//
