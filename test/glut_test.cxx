//
// GLUT test program for the Fast Light Tool Kit (FLTK).
//
// Provided by Brian Schack (STR #3458, see "big.cxx").
// Copyright 2023 by Bill Spitzak and others.
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

// Thanks to the original author Brian Schack for this test program.
// This program has been extended to test several GLUT functions with
// (1) a valid and (2) an invalid (destroyed) GLUT window.
// The test program opens two GLUT windows, runs the tests, prints
// diagnostics on stdout, and exits immediately. This is intended.

// To compile (examples with different GLUT implementations)
// as of Apr 03, 2018 provided Brian Schack, slightly modified
//
// macOS GLUT:
//   g++ -o glut_test glut_test.cxx -framework OpenGL -framework GLUT
//
// FreeGLUT (on macOS, with Macport's FreeGLUT):
//   g++ -o glut_test glut_test.cxx -framework OpenGL -I/opt/local/include -L/opt/local/lib -lglut
//
// FLTK
//   fltk-config --use-gl --compile glut_test.cxx

// Enable one of the following two #include's depending on the GLUT implementation

// #include <GLUT/glut.h>               // GLUT and FreeGLUT version
#include <FL/glut.H>        // FLTK version

#include <stdio.h>

// Empty callback functions for testing.
void displayFunc() {}
void reshapeFunc(int w, int h) {}
void keyboardFunc(unsigned char key, int x, int y) {}
void mouseFunc(int b, int state, int x, int y) {}
void motionFunc(int x, int y) {}
void passiveMotionFunc(int x, int y) {}
void entryFunc(int s) {}
void visibilityFunc(int s) {}
void idleFunc() {}
void timerFunc(int value) {}
void menuStateFunc(int state) {}
void menuStatusFunc(int status, int x, int y) {}
void specialFunc(int key, int x, int y) {}
void overlayDisplayFunc() {}

int main(int argc, char **argv) {
  glutInit(&argc, argv);

  // Create 2 windows.
  int win1 = glutCreateWindow("Window 1");
  int win2 = glutCreateWindow("Window 2");
  printf("Window 1 created, number = %d\n", win1);
  printf("Window 2 created, number = %d\n", win2);

  // Run tests twice, with (1) a valid and (2) an invalid current window

  for (int i = 0; i < 2; i++) {

    // Find out which window is current.
    int current = glutGetWindow();
    printf("Window %d is current\n", current);

    // Ask GLUT to redisplay things.
    glutPostRedisplay();

    // Set window title
    glutSetWindowTitle((char *)"Non-existent");

    // Set icon title
    glutSetIconTitle((char *)"Non-existent");

    // Position window
    glutPositionWindow(10, 20);

    // Reshape window
    glutReshapeWindow(100, 200);

    // Pop window
    glutPopWindow();

    // Iconify window
    glutIconifyWindow();

    // Show window
    glutShowWindow();

    // Hide window
    glutHideWindow();

    // Go to full screen mode
    glutFullScreen();

    // Set the cursor
    glutSetCursor(GLUT_CURSOR_INFO);

    // Establish an overlay
    glutEstablishOverlay();

    // Remove overlay
    glutRemoveOverlay();

    // Choose a layer
    glutUseLayer(GLUT_NORMAL);
    glutUseLayer(GLUT_OVERLAY);

    // Post display on a layer
    glutPostOverlayRedisplay();

    // Show overlay
    glutShowOverlay();

    // Hide overlay
    glutHideOverlay();

    // Attach a menu
    glutAttachMenu(0);

    // Detach a menu
    glutDetachMenu(0);

    // Specify callbacks
    glutDisplayFunc(displayFunc);
    glutReshapeFunc(reshapeFunc);
    glutKeyboardFunc(keyboardFunc);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);
    glutPassiveMotionFunc(passiveMotionFunc);
    glutEntryFunc(entryFunc);
    glutVisibilityFunc(visibilityFunc);
    glutIdleFunc(idleFunc);
    glutTimerFunc(1000, timerFunc, 42);
    glutMenuStateFunc(menuStateFunc);
    glutMenuStatusFunc(menuStatusFunc);
    glutSpecialFunc(specialFunc);
    glutOverlayDisplayFunc(overlayDisplayFunc);

    // Swap buffers
    glutSwapBuffers();

    // GLUT gets
    printf("GLUT_WINDOW_X = %d\n", glutGet(GLUT_WINDOW_X));
    printf("GLUT_WINDOW_Y = %d\n", glutGet(GLUT_WINDOW_Y));
    printf("GLUT_WINDOW_WIDTH = %d\n", glutGet(GLUT_WINDOW_WIDTH));
    printf("GLUT_WINDOW_HEIGHT = %d\n", glutGet(GLUT_WINDOW_HEIGHT));
    printf("GLUT_WINDOW_PARENT = %d\n", glutGet(GLUT_WINDOW_PARENT));

    // GLUT layer gets
    printf("GLUT_OVERLAY_POSSIBLE = %d\n", glutLayerGet(GLUT_OVERLAY_POSSIBLE));
    printf("GLUT_NORMAL_DAMAGED = %d\n", glutLayerGet(GLUT_NORMAL_DAMAGED));

    // Destroy the current window - this sets glut_window to NULL
    printf("Destroy the current window (%d)\n\n", glutGetWindow());
    glutDestroyWindow(current);

  } // loop with current window

  printf("All tests done, exiting.\n");
  return 0;
}
