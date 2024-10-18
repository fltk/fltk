//
// OpenGL puzzle demo for the Fast Light Tool Kit (FLTK).
//
// This is a GLUT demo program to demonstrate fltk's GLUT emulation.
// Search for "fltk" to find all the changes
//
// Copyright 1998-2024 by Bill Spitzak and others.
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

// Convenience options 'n' and ' ' and command line switch '-n' added for FLTK

// this block added for fltk's distribution so it will compile w/o OpenGL:
#include <config.h>
#if !HAVE_GL || !HAVE_GL_GLU_H
#include <FL/Fl.H>
#include <FL/fl_message.H>
int main(int, char**) {
  fl_alert("This demo does not work without GL and GLU");
  return 1;
}
#else
// end of added block

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <FL/glut.H>    // changed for fltk
#include <FL/glu.h>     // added for fltk
#include "trackball.c"  // changed from trackball.h for fltk

#define WIDTH 4
#define HEIGHT 5
#define PIECES 10
#define OFFSETX -2.0f
#define OFFSETY -2.5f
#define OFFSETZ -0.5f

typedef char Config[HEIGHT][WIDTH];

struct puzzle {
  struct puzzle *backptr;
  struct puzzle *solnptr;
  Config pieces;
  struct puzzle *next;
  unsigned hashvalue;
};

#define HASHSIZE 10691

struct puzzlelist {
  struct puzzle *puzzle;
  struct puzzlelist *next;
};

static char convert[PIECES + 1] =
{0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 4};

static unsigned char colors[PIECES + 1][3] =
{
  {0, 0, 0},
  {255, 255, 127},
  {255, 255, 127},
  {255, 255, 127},
  {255, 255, 127},
  {255, 127, 255},
  {255, 127, 255},
  {255, 127, 255},
  {255, 127, 255},
  {255, 127, 127},
  {255, 255, 255},
};

void changeState(void);
void animate(void);

static struct puzzle *hashtable[HASHSIZE];
static struct puzzle *startPuzzle;
static struct puzzlelist *puzzles;
static struct puzzlelist *lastentry;

int curX, curY, visible;

#define MOVE_SPEED 0.2f
static unsigned char movingPiece;
static float move_x, move_y;
static float curquat[4];
static int doubleBuffer = 1;
static int depth = 1;

static char xsize[PIECES + 1] =
{0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2};
static char ysize[PIECES + 1] =
{0, 1, 1, 1, 1, 2, 2, 2, 2, 1, 2};
static float zsize[PIECES + 1] =
{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0.6f};

static Config startConfig =
{
  {8, 10, 10, 7},
  {8, 10, 10, 7},
  {6, 9, 9, 5},
  {6, 4, 3, 5},
  {2, 0, 0, 1}
};

static Config thePuzzle =
{
  {8, 10, 10, 7},
  {8, 10, 10, 7},
  {6, 9, 9, 5},
  {6, 4, 3, 5},
  {2, 0, 0, 1}
};

static int xadds[4] =
{-1, 0, 1, 0};
static int yadds[4] =
{0, -1, 0, 1};

static long W = 400, H = 300;
static GLint viewport[4];

#define srandom srand
#define random() (rand() >> 2)

unsigned
hash(Config config)
{
  int i, j, value;

  value = 0;
  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      value = value + convert[(int)config[i][j]];
      value *= 6;
    }
  }
  return (value);
}

int
solution(Config config)
{
  if (config[4][1] == 10 && config[4][2] == 10)
    return (1);
  return (0);
}

float boxcoords[][3] =
{
  {0.2f, 0.2f, 0.9f},
  {0.8f, 0.2f, 0.9f},
  {0.8f, 0.8f, 0.9f},
  {0.2f, 0.8f, 0.9f},
  {0.2f, 0.1f, 0.8f},
  {0.8f, 0.1f, 0.8f},
  {0.9f, 0.2f, 0.8f},
  {0.9f, 0.8f, 0.8f},
  {0.8f, 0.9f, 0.8f},
  {0.2f, 0.9f, 0.8f},
  {0.1f, 0.8f, 0.8f},
  {0.1f, 0.2f, 0.8f},
  {0.2f, 0.1f, 0.2f},
  {0.8f, 0.1f, 0.2f},
  {0.9f, 0.2f, 0.2f},
  {0.9f, 0.8f, 0.2f},
  {0.8f, 0.9f, 0.2f},
  {0.2f, 0.9f, 0.2f},
  {0.1f, 0.8f, 0.2f},
  {0.1f, 0.2f, 0.2f},
  {0.2f, 0.2f, 0.1f},
  {0.8f, 0.2f, 0.1f},
  {0.8f, 0.8f, 0.1f},
  {0.2f, 0.8f, 0.1f},
};

float boxnormals[][3] =
{
  {0, 0, 1},            /* 0 */
  {0, 1, 0},
  {1, 0, 0},
  {0, 0, -1},
  {0, -1, 0},
  {-1, 0, 0},
  {0.7071f, 0.7071f, 0.0000f},  /* 6 */
  {0.7071f, -0.7071f, 0.0000f},
  {-0.7071f, 0.7071f, 0.0000f},
  {-0.7071f, -0.7071f, 0.0000f},
  {0.7071f, 0.0000f, 0.7071f},  /* 10 */
  {0.7071f, 0.0000f, -0.7071f},
  {-0.7071f, 0.0000f, 0.7071f},
  {-0.7071f, 0.0000f, -0.7071f},
  {0.0000f, 0.7071f, 0.7071f},  /* 14 */
  {0.0000f, 0.7071f, -0.7071f},
  {0.0000f, -0.7071f, 0.7071f},
  {0.0000f, -0.7071f, -0.7071f},
  {0.5774f, 0.5774f, 0.5774f},  /* 18 */
  {0.5774f, 0.5774f, -0.5774f},
  {0.5774f, -0.5774f, 0.5774f},
  {0.5774f, -0.5774f, -0.5774f},
  {-0.5774f, 0.5774f, 0.5774f},
  {-0.5774f, 0.5774f, -0.5774f},
  {-0.5774f, -0.5774f, 0.5774f},
  {-0.5774f, -0.5774f, -0.5774f},
};

int boxfaces[][4] =
{
  {0, 1, 2, 3},         /* 0 */
  {9, 8, 16, 17},
  {6, 14, 15, 7},
  {20, 23, 22, 21},
  {12, 13, 5, 4},
  {19, 11, 10, 18},
  {7, 15, 16, 8},       /* 6 */
  {13, 14, 6, 5},
  {18, 10, 9, 17},
  {19, 12, 4, 11},
  {1, 6, 7, 2},         /* 10 */
  {14, 21, 22, 15},
  {11, 0, 3, 10},
  {20, 19, 18, 23},
  {3, 2, 8, 9},         /* 14 */
  {17, 16, 22, 23},
  {4, 5, 1, 0},
  {20, 21, 13, 12},
  {2, 7, 8, -1},        /* 18 */
  {16, 15, 22, -1},
  {5, 6, 1, -1},
  {13, 21, 14, -1},
  {10, 3, 9, -1},
  {18, 17, 23, -1},
  {11, 4, 0, -1},
  {20, 12, 19, -1},
};

#define NBOXFACES (sizeof(boxfaces)/sizeof(boxfaces[0]))

/* Draw a box.  Bevel as desired. */
void
drawBox(int piece, float xoff, float yoff)
{
  int xlen, ylen;
  int i, k;
  float x, y, z;
  float zlen;
  float *v;

  xlen = xsize[piece];
  ylen = ysize[piece];
  zlen = zsize[piece];

  glColor3ubv(colors[piece]);
  glBegin(GL_QUADS);
  for (i = 0; i < 18; i++) {
    glNormal3fv(boxnormals[i]);
    for (k = 0; k < 4; k++) {
      if (boxfaces[i][k] == -1)
        continue;
      v = boxcoords[boxfaces[i][k]];
      x = v[0] + OFFSETX;
      if (v[0] > 0.5)
        x += xlen - 1;
      y = v[1] + OFFSETY;
      if (v[1] > 0.5)
        y += ylen - 1;
      z = v[2] + OFFSETZ;
      if (v[2] > 0.5)
        z += zlen - 1;
      glVertex3f(xoff + x, yoff + y, z);
    }
  }
  glEnd();
  glBegin(GL_TRIANGLES);
  for (i = 18; i < int(NBOXFACES); i++) {
    glNormal3fv(boxnormals[i]);
    for (k = 0; k < 3; k++) {
      if (boxfaces[i][k] == -1)
        continue;
      v = boxcoords[boxfaces[i][k]];
      x = v[0] + OFFSETX;
      if (v[0] > 0.5)
        x += xlen - 1;
      y = v[1] + OFFSETY;
      if (v[1] > 0.5)
        y += ylen - 1;
      z = v[2] + OFFSETZ;
      if (v[2] > 0.5)
        z += zlen - 1;
      glVertex3f(xoff + x, yoff + y, z);
    }
  }
  glEnd();
}

float containercoords[][3] =
{
  {-0.1f, -0.1f, 1.0f},
  {-0.1f, -0.1f, -0.1f},
  {4.1f, -0.1f, -0.1f},
  {4.1f, -0.1f, 1.0f},
  {1.0f, -0.1f, 0.6f},     /* 4 */
  {3.0f, -0.1f, 0.6f},
  {1.0f, -0.1f, 0.0f},
  {3.0f, -0.1f, 0.0f},
  {1.0f, 0.0f, 0.0f},      /* 8 */
  {3.0f, 0.0f, 0.0f},
  {3.0f, 0.0f, 0.6f},
  {1.0f, 0.0f, 0.6f},
  {0.0f, 0.0f, 1.0f},      /* 12 */
  {4.0f, 0.0f, 1.0f},
  {4.0f, 0.0f, 0.0f},
  {0.0f, 0.0f, 0.0f},
  {0.0f, 5.0f, 0.0f},      /* 16 */
  {0.0f, 5.0f, 1.0f},
  {4.0f, 5.0f, 1.0f},
  {4.0f, 5.0f, 0.0f},
  {-0.1f, 5.1f, -0.1f},    /* 20 */
  {4.1f, 5.1f, -0.1f},
  {4.1f, 5.1f, 1.0f},
  {-0.1f, 5.1f, 1.0f},
};

float containernormals[][3] =
{
  {0, -1, 0},
  {0, -1, 0},
  {0, -1, 0},
  {0, -1, 0},
  {0, -1, 0},
  {0, 1, 0},
  {0, 1, 0},
  {0, 1, 0},
  {1, 0, 0},
  {1, 0, 0},
  {1, 0, 0},
  {-1, 0, 0},
  {-1, 0, 0},
  {-1, 0, 0},
  {0, 1, 0},
  {0, 0, -1},
  {0, 0, -1},
  {0, 0, 1},
  {0, 0, 1},
  {0, 0, 1},
  {0, 0, 1},
  {0, 0, 1},
  {0, 0, 1},
  {0, 0, 1},
};

int containerfaces[][4] =
{
  {1, 6, 4, 0},
  {0, 4, 5, 3},
  {1, 2, 7, 6},
  {7, 2, 3, 5},
  {16, 19, 18, 17},

  {23, 22, 21, 20},
  {12, 11, 8, 15},
  {10, 13, 14, 9},

  {15, 16, 17, 12},
  {2, 21, 22, 3},
  {6, 8, 11, 4},

  {1, 0, 23, 20},
  {14, 13, 18, 19},
  {9, 7, 5, 10},

  {12, 13, 10, 11},

  {1, 20, 21, 2},
  {4, 11, 10, 5},

  {15, 8, 19, 16},
  {19, 8, 9, 14},
  {8, 6, 7, 9},
  {0, 3, 13, 12},
  {13, 3, 22, 18},
  {18, 22, 23, 17},
  {17, 23, 0, 12},
};

#define NCONTFACES (sizeof(containerfaces)/sizeof(containerfaces[0]))

/* Draw the container */
void
drawContainer(void)
{
  int i, k;
  float *v;

  /* Y is reversed here because the model has it reversed */

  /* Arbitrary bright wood-like color */
  glColor3ub(209, 103, 23);
  glBegin(GL_QUADS);
  for (i = 0; i < int(NCONTFACES); i++) {
    v = containernormals[i];
    glNormal3f(v[0], -v[1], v[2]);
    for (k = 3; k >= 0; k--) {
      v = containercoords[containerfaces[i][k]];
      glVertex3f(v[0] + OFFSETX, -(v[1] + OFFSETY), v[2] + OFFSETZ);
    }
  }
  glEnd();
}

void
drawAll(void)
{
  int i, j;
  int piece;
  char done[PIECES + 1];
  float m[4][4];

  build_rotmatrix(m, curquat);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(0, 0, -10);
  glMultMatrixf(&(m[0][0]));
  glRotatef(180, 0, 0, 1);

  if (depth) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  } else {
    glClear(GL_COLOR_BUFFER_BIT);
  }
  for (i = 1; i <= PIECES; i++) {
    done[i] = 0;
  }
  glLoadName(0);
  drawContainer();
  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      piece = thePuzzle[i][j];
      if (piece == 0)
        continue;
      if (done[piece])
        continue;
      done[piece] = 1;
      glLoadName(piece);
      if (piece == movingPiece) {
        drawBox(piece, move_x, move_y);
      } else {
        drawBox(piece, float(j), float(i));
      }
    }
  }
}

void
redraw(void)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45, viewport[2]*1.0/viewport[3], 0.1, 100.0);

  drawAll();

  if (doubleBuffer)
    glutSwapBuffers();
  else
    glFinish();
}

void
solidifyChain(struct puzzle *puzzle)
{
  int i;
  char buf[256];

  i = 0;
  while (puzzle->backptr) {
    i++;
    puzzle->backptr->solnptr = puzzle;
    puzzle = puzzle->backptr;
  }
  snprintf(buf, 256, "%d moves to complete!", i);
  glutSetWindowTitle(buf);
}

int
addConfig(Config config, struct puzzle *back)
{
  unsigned hashvalue;
  struct puzzle *newpiece;
  struct puzzlelist *newlistentry;

  hashvalue = hash(config);

  newpiece = hashtable[hashvalue % HASHSIZE];
  while (newpiece != NULL) {
    if (newpiece->hashvalue == hashvalue) {
      int i, j;

      for (i = 0; i < WIDTH; i++) {
        for (j = 0; j < HEIGHT; j++) {
          if (convert[(int)config[j][i]] !=
            convert[(int)newpiece->pieces[j][i]])
            goto nomatch;
        }
      }
      return 0;
    }
  nomatch:
    newpiece = newpiece->next;
  }

  newpiece = (struct puzzle *) malloc(sizeof(struct puzzle));
  newpiece->next = hashtable[hashvalue % HASHSIZE];
  newpiece->hashvalue = hashvalue;
  memcpy(newpiece->pieces, config, HEIGHT * WIDTH);
  newpiece->backptr = back;
  newpiece->solnptr = NULL;
  hashtable[hashvalue % HASHSIZE] = newpiece;

  newlistentry = (struct puzzlelist *) malloc(sizeof(struct puzzlelist));
  newlistentry->puzzle = newpiece;
  newlistentry->next = NULL;

  if (lastentry) {
    lastentry->next = newlistentry;
  } else {
    puzzles = newlistentry;
  }
  lastentry = newlistentry;

  if (back == NULL) {
    startPuzzle = newpiece;
  }
  if (solution(config)) {
    solidifyChain(newpiece);
    return 1;
  }
  return 0;
}

/* Checks if a space can move */
int
canmove0(Config pieces, int x, int y, int dir, Config newpieces)
{
  char piece;
  int xadd, yadd;
  int l, m;

  xadd = xadds[dir];
  yadd = yadds[dir];

  if (x + xadd < 0 || x + xadd >= WIDTH ||
    y + yadd < 0 || y + yadd >= HEIGHT)
    return 0;
  piece = pieces[y + yadd][x + xadd];
  if (piece == 0)
    return 0;
  memcpy(newpieces, pieces, HEIGHT * WIDTH);
  for (l = 0; l < WIDTH; l++) {
    for (m = 0; m < HEIGHT; m++) {
      if (newpieces[m][l] == piece)
        newpieces[m][l] = 0;
    }
  }
  xadd = -xadd;
  yadd = -yadd;
  for (l = 0; l < WIDTH; l++) {
    for (m = 0; m < HEIGHT; m++) {
      if (pieces[m][l] == piece) {
        int newx, newy;

        newx = l + xadd;
        newy = m + yadd;
        if (newx < 0 || newx >= WIDTH ||
          newy < 0 || newy >= HEIGHT)
          return 0;
        if (newpieces[newy][newx] != 0)
          return 0;
        newpieces[newy][newx] = piece;
      }
    }
  }
  return 1;
}

/* Checks if a piece can move */
int
canmove(Config pieces, int x, int y, int dir, Config newpieces)
{
  int xadd, yadd;

  xadd = xadds[dir];
  yadd = yadds[dir];

  if (x + xadd < 0 || x + xadd >= WIDTH ||
    y + yadd < 0 || y + yadd >= HEIGHT)
    return 0;
  if (pieces[y + yadd][x + xadd] == pieces[y][x]) {
    return canmove(pieces, x + xadd, y + yadd, dir, newpieces);
  }
  if (pieces[y + yadd][x + xadd] != 0)
    return 0;
  return canmove0(pieces, x + xadd, y + yadd, (dir + 2) % 4, newpieces);
}

int
generateNewConfigs(struct puzzle *puzzle)
{
  int i, j, k;
  Config pieces;
  Config newpieces;

  memcpy(pieces, puzzle->pieces, HEIGHT * WIDTH);
  for (i = 0; i < WIDTH; i++) {
    for (j = 0; j < HEIGHT; j++) {
      if (pieces[j][i] == 0) {
        for (k = 0; k < 4; k++) {
          if (canmove0(pieces, i, j, k, newpieces)) {
            if (addConfig(newpieces, puzzle))
              return 1;
          }
        }
      }
    }
  }
  return 0;
}

void
freeSolutions(void)
{
  struct puzzlelist *nextpuz;
  struct puzzle *puzzle, *next;
  int i;

  while (puzzles) {
    nextpuz = puzzles->next;
    free((char *) puzzles);
    puzzles = nextpuz;
  }
  lastentry = NULL;
  for (i = 0; i < HASHSIZE; i++) {
    puzzle = hashtable[i];
    hashtable[i] = NULL;
    while (puzzle) {
      next = puzzle->next;
      free((char *) puzzle);
      puzzle = next;
    }
  }
  startPuzzle = NULL;
}

int
continueSolving(void)
{
  struct puzzle *nextpuz;
  int i, j;
  int movedPiece;
  int movedir;
  int fromx, fromy;
  int tox, toy;

  if (startPuzzle == NULL)
    return 0;
  if (startPuzzle->solnptr == NULL) {
    freeSolutions();
    return 0;
  }
  nextpuz = startPuzzle->solnptr;
  movedPiece = 0;
  movedir = 0;
  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      if (startPuzzle->pieces[i][j] != nextpuz->pieces[i][j]) {
        if (startPuzzle->pieces[i][j]) {
          movedPiece = startPuzzle->pieces[i][j];
          fromx = j;
          fromy = i;
          if (i < HEIGHT - 1 && nextpuz->pieces[i + 1][j] == movedPiece) {
            movedir = 3;
          } else {
            movedir = 2;
          }
          goto found_piece;
        } else {
          movedPiece = nextpuz->pieces[i][j];
          if (i < HEIGHT - 1 &&
            startPuzzle->pieces[i + 1][j] == movedPiece) {
            fromx = j;
            fromy = i + 1;
            movedir = 1;
          } else {
            fromx = j + 1;
            fromy = i;
            movedir = 0;
          }
          goto found_piece;
        }
      }
    }
  }
  glutSetWindowTitle((char *)"What!  No change?");
  freeSolutions();
  return 0;

found_piece:
  if (!movingPiece) {
    movingPiece = movedPiece;
    move_x = float(fromx);
    move_y = float(fromy);
  }
  move_x += xadds[movedir] * MOVE_SPEED;
  move_y += yadds[movedir] * MOVE_SPEED;

  tox = fromx + xadds[movedir];
  toy = fromy + yadds[movedir];

  if (move_x > tox - MOVE_SPEED / 2 && move_x < tox + MOVE_SPEED / 2 &&
    move_y > toy - MOVE_SPEED / 2 && move_y < toy + MOVE_SPEED / 2) {
    startPuzzle = nextpuz;
    movingPiece = 0;
  }
  memcpy(thePuzzle, startPuzzle->pieces, HEIGHT * WIDTH);
  changeState();
  return 1;
}

int
solvePuzzle(void)
{
  struct puzzlelist *nextpuz;
  char buf[256];
  int i;

  if (solution(thePuzzle)) {
    glutSetWindowTitle((char *)"Puzzle already solved!");
    return 0;
  }
  addConfig(thePuzzle, NULL);
  i = 0;

  while (puzzles) {
    i++;
    if (generateNewConfigs(puzzles->puzzle))
      break;
    nextpuz = puzzles->next;
    free((char *) puzzles);
    puzzles = nextpuz;
  }
  if (puzzles == NULL) {
    freeSolutions();
    snprintf(buf, 256, "I can't solve it! (%d positions examined)", i);
    glutSetWindowTitle(buf);
    return 1;
  }
  return 1;
}

int
selectPiece(int mousex, int mousey)
{
  long hits;
  GLuint selectBuf[1024];
  GLuint closest;
  GLuint dist;

  glSelectBuffer(1024, selectBuf);
  (void) glRenderMode(GL_SELECT);
  glInitNames();

  /* Because LoadName() won't work with no names on the stack */
  glPushName(0);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPickMatrix(mousex, H - mousey, 4, 4, viewport);
  gluPerspective(45, viewport[2]*1.0/viewport[3], 0.1, 100.0);

  drawAll();

  hits = glRenderMode(GL_RENDER);
  if (hits <= 0) {
    return 0;
  }
  closest = 0;
  dist = 0xFFFFFFFFU; //2147483647;
  while (hits) {
    if (selectBuf[(hits - 1) * 4 + 1] < dist) {
      dist = selectBuf[(hits - 1) * 4 + 1];
      closest = selectBuf[(hits - 1) * 4 + 3];
    }
    hits--;
  }
  return closest;
}

void
nukePiece(int piece)
{
  int i, j;

  for (i = 0; i < HEIGHT; i++) {
    for (j = 0; j < WIDTH; j++) {
      if (thePuzzle[i][j] == piece) {
        thePuzzle[i][j] = 0;
      }
    }
  }
}

void
multMatrices(const GLfloat a[16], const GLfloat b[16], GLfloat r[16])
{
  int i, j;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      r[i * 4 + j] =
        a[i * 4 + 0] * b[0 * 4 + j] +
        a[i * 4 + 1] * b[1 * 4 + j] +
        a[i * 4 + 2] * b[2 * 4 + j] +
        a[i * 4 + 3] * b[3 * 4 + j];
    }
  }
}

void
makeIdentity(GLfloat m[16])
{
  m[0 + 4 * 0] = 1;
  m[0 + 4 * 1] = 0;
  m[0 + 4 * 2] = 0;
  m[0 + 4 * 3] = 0;
  m[1 + 4 * 0] = 0;
  m[1 + 4 * 1] = 1;
  m[1 + 4 * 2] = 0;
  m[1 + 4 * 3] = 0;
  m[2 + 4 * 0] = 0;
  m[2 + 4 * 1] = 0;
  m[2 + 4 * 2] = 1;
  m[2 + 4 * 3] = 0;
  m[3 + 4 * 0] = 0;
  m[3 + 4 * 1] = 0;
  m[3 + 4 * 2] = 0;
  m[3 + 4 * 3] = 1;
}

/*
   ** inverse = invert(src)
 */
int
invertMatrix(const GLfloat src[16], GLfloat inverse[16])
{
  int i, j, k, swap;
  float t;
  GLfloat temp[4][4];

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      temp[i][j] = src[i * 4 + j];
    }
  }
  makeIdentity(inverse);

  for (i = 0; i < 4; i++) {
    /*
       ** Look for largest element in column */
    swap = i;
    for (j = i + 1; j < 4; j++) {
      if (fabs(temp[j][i]) > fabs(temp[i][i])) {
        swap = j;
      }
    }

    if (swap != i) {
      /*
         ** Swap rows. */
      for (k = 0; k < 4; k++) {
        t = temp[i][k];
        temp[i][k] = temp[swap][k];
        temp[swap][k] = t;

        t = inverse[i * 4 + k];
        inverse[i * 4 + k] = inverse[swap * 4 + k];
        inverse[swap * 4 + k] = t;
      }
    }
    if (temp[i][i] == 0) {
      /*
         ** No non-zero pivot.  The matrix is singular, which
         shouldn't ** happen.  This means the user gave us a
         bad matrix. */
      return 0;
    }
    t = temp[i][i];
    for (k = 0; k < 4; k++) {
      temp[i][k] /= t;
      inverse[i * 4 + k] /= t;
    }
    for (j = 0; j < 4; j++) {
      if (j != i) {
        t = temp[j][i];
        for (k = 0; k < 4; k++) {
          temp[j][k] -= temp[i][k] * t;
          inverse[j * 4 + k] -= inverse[i * 4 + k] * t;
        }
      }
    }
  }
  return 1;
}

/*
   ** This is a screwball function.  What it does is the following:
   ** Given screen x and y coordinates, compute the corresponding object space
   **   x and y coordinates given that the object space z is 0.9 + OFFSETZ.
   ** Since the tops of (most) pieces are at z = 0.9 + OFFSETZ, we use that
   **   number.
 */
int
computeCoords(int piece, int mousex, int mousey,
  GLfloat * selx, GLfloat * sely)
{
  GLfloat modelMatrix[16];
  GLfloat projMatrix[16];
  GLfloat finalMatrix[16];
  GLfloat in[4];
  GLfloat a, b, c, d;
  GLfloat top, bot;
  GLfloat z;
  GLfloat w;
  GLfloat height;

  if (piece == 0)
    return 0;
  height = zsize[piece] - 0.1f + OFFSETZ;

  glGetFloatv(GL_PROJECTION_MATRIX, projMatrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, modelMatrix);
  multMatrices(modelMatrix, projMatrix, finalMatrix);
  if (!invertMatrix(finalMatrix, finalMatrix))
    return 0;

  in[0] = (2.0f * (mousex - viewport[0]) / viewport[2]) - 1;
  in[1] = (2.0f * ((H - mousey) - viewport[1]) / viewport[3]) - 1;

  a = in[0] * finalMatrix[0 * 4 + 2] +
    in[1] * finalMatrix[1 * 4 + 2] +
    finalMatrix[3 * 4 + 2];
  b = finalMatrix[2 * 4 + 2];
  c = in[0] * finalMatrix[0 * 4 + 3] +
    in[1] * finalMatrix[1 * 4 + 3] +
    finalMatrix[3 * 4 + 3];
  d = finalMatrix[2 * 4 + 3];

  /*
     ** Ok, now we need to solve for z: **   (a + b z) / (c + d

     z) = height. ** ("height" is the height in object space we

     want to solve z for) ** ** ==>  a + b z = height c +
     height d z **      bz - height d z = height c - a ** z =
     (height c - a) / (b - height d) */
  top = height * c - a;
  bot = b - height * d;
  if (bot == 0.0)
    return 0;

  z = top / bot;

  /*
     ** Ok, no problem. ** Now we solve for x and y.  We know
     that w = c + d z, so we compute it. */
  w = c + d * z;

  /*
     ** Now for x and y: */
  *selx = (in[0] * finalMatrix[0 * 4 + 0] +
    in[1] * finalMatrix[1 * 4 + 0] +
    z * finalMatrix[2 * 4 + 0] +
    finalMatrix[3 * 4 + 0]) / w - OFFSETX;
  *sely = (in[0] * finalMatrix[0 * 4 + 1] +
    in[1] * finalMatrix[1 * 4 + 1] +
    z * finalMatrix[2 * 4 + 1] +
    finalMatrix[3 * 4 + 1]) / w - OFFSETY;
  return 1;
}

static int selected;
static int selectx, selecty;
static float selstartx, selstarty;

void
grabPiece(int piece, float selx, float sely)
{
  int hit;

  selectx = int(selx);
  selecty = int(sely);
  if (selectx < 0 || selecty < 0 || selectx >= WIDTH || selecty >= HEIGHT) {
    return;
  }
  hit = thePuzzle[selecty][selectx];
  if (hit != piece)
    return;
  if (hit) {
    movingPiece = hit;
    while (selectx > 0 && thePuzzle[selecty][selectx - 1] == movingPiece) {
      selectx--;
    }
    while (selecty > 0 && thePuzzle[selecty - 1][selectx] == movingPiece) {
      selecty--;
    }
    move_x = float(selectx);
    move_y = float(selecty);
    selected = 1;
    selstartx = selx;
    selstarty = sely;
  } else {
    selected = 0;
  }
  changeState();
}

void
moveSelection(float selx, float sely)
{
  float deltax, deltay;
  int dir;
  Config newpieces;

  if (!selected)
    return;
  deltax = selx - selstartx;
  deltay = sely - selstarty;

  if (fabs(deltax) > fabs(deltay)) {
    deltay = 0;
    if (deltax > 0) {
      if (deltax > 1)
        deltax = 1;
      dir = 2;
    } else {
      if (deltax < -1)
        deltax = -1;
      dir = 0;
    }
  } else {
    deltax = 0;
    if (deltay > 0) {
      if (deltay > 1)
        deltay = 1;
      dir = 3;
    } else {
      if (deltay < -1)
        deltay = -1;
      dir = 1;
    }
  }
  if (canmove(thePuzzle, selectx, selecty, dir, newpieces)) {
    move_x = deltax + selectx;
    move_y = deltay + selecty;
    if (deltax > 0.5) {
      memcpy(thePuzzle, newpieces, HEIGHT * WIDTH);
      selectx++;
      selstartx++;
    } else if (deltax < -0.5) {
      memcpy(thePuzzle, newpieces, HEIGHT * WIDTH);
      selectx--;
      selstartx--;
    } else if (deltay > 0.5) {
      memcpy(thePuzzle, newpieces, HEIGHT * WIDTH);
      selecty++;
      selstarty++;
    } else if (deltay < -0.5) {
      memcpy(thePuzzle, newpieces, HEIGHT * WIDTH);
      selecty--;
      selstarty--;
    }
  } else {
    if (deltay > 0 && thePuzzle[selecty][selectx] == 10 &&
      selectx == 1 && selecty == 3) {
      /* Allow visual movement of solution piece outside of the

         box */
      move_x = float(selectx);
      move_y = sely - selstarty + selecty;
    } else {
      move_x = float(selectx);
      move_y = float(selecty);
    }
  }
}

void
dropSelection(void)
{
  if (!selected)
    return;
  movingPiece = 0;
  selected = 0;
  changeState();
}

static int left_mouse, middle_mouse;
static int mousex, mousey;
static int solving;
static int spinning;
static int enable_spinning = 1;
static float lastquat[4];
static int sel_piece;

static void
Reshape(int width, int height)
{

  W = width;
  H = height;
  glViewport(0, 0, (GLsizei)W, (GLsizei)H);
  glGetIntegerv(GL_VIEWPORT, viewport);
}

void
toggleSolve(void)
{
    if (solving) {
      freeSolutions();
      solving = 0;
      glutChangeToMenuEntry(1, (char *)"Solving", 1);
      glutSetWindowTitle((char *)"glpuzzle");
      movingPiece = 0;
    } else {
      glutChangeToMenuEntry(1, (char *)"Stop solving", 1);
      glutSetWindowTitle((char *)"Solving...");
      if (solvePuzzle()) {
        solving = 1;
      }
    }
    changeState();
    glutPostRedisplay();
}

void reset_position(void)
{
    spinning = 0;
    trackball(curquat, 0.0, 0.0, 0.0, 0.0); // reset position
    glutIdleFunc(animate);
}

void reset(void)
{
    reset_position();
    if (solving) {
      freeSolutions();
      solving = 0;
      glutChangeToMenuEntry(1, (char *)"Solving", 1);
      glutSetWindowTitle((char *)"glpuzzle");
      movingPiece = 0;
      changeState();
    }
    memcpy(thePuzzle, startConfig, HEIGHT * WIDTH);
    glutPostRedisplay();
}

void
keyboard(unsigned char c, int x, int y)
{
  int piece;

  switch (c) {
  case 27:
    exit(0);
    break;
  case ' ':
  case 'n':
  case 'N':
    reset_position();
    break;
  case 'D':
  case 'd':
    if (solving) {
      freeSolutions();
      solving = 0;
      glutChangeToMenuEntry(1, (char *)"Solving", 1);
      glutSetWindowTitle((char *)"glpuzzle");
      movingPiece = 0;
      changeState();
    }
    piece = selectPiece(x, y);
    if (piece) {
      nukePiece(piece);
    }
    glutPostRedisplay();
    break;
  case 'R':
  case 'r':
    reset();
    break;
  case 'S':
  case 's':
    toggleSolve();
    break;
  case 'b':
  case 'B':
    depth = 1 - depth;
    if (depth) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
    glutPostRedisplay();
    break;
  default:
    break;
  }
}

void
motion(int x, int y)
{
  float selx, sely;

  if (middle_mouse && !left_mouse) {
    if (mousex != x || mousey != y) {
      trackball(lastquat,
        (2.0f*mousex - W) / W,
        (H - 2.0f*mousey) / H,
        (2.0f*x - W) / W,
        (H - 2.0f*y) / H);
      spinning = enable_spinning; // 1 = yes, 0 = disabled (commandline -n)
    } else {
      spinning = 0;
    }
    changeState();
  } else {
    computeCoords(sel_piece, x, y, &selx, &sely);
    moveSelection(selx, sely);
  }
  mousex = x;
  mousey = y;
  glutPostRedisplay();
}

void
mouse(int b, int s, int x, int y)
{
  float selx, sely;

  mousex = x;
  mousey = y;
  curX = x;
  curY = y;
  if (s == GLUT_DOWN) {
    switch (b) {
    case GLUT_LEFT_BUTTON:
      if (solving) {
        freeSolutions();
        solving = 0;
      glutChangeToMenuEntry(1, (char *)"Solving", 1);
        glutSetWindowTitle((char *)"glpuzzle");
        movingPiece = 0;
      }
      left_mouse = GL_TRUE;
      sel_piece = selectPiece(mousex, mousey);
      if (!sel_piece) {
      left_mouse = GL_FALSE;
      middle_mouse = GL_TRUE; // let it rotate object
      } else if (computeCoords(sel_piece, mousex, mousey, &selx, &sely)) {
        grabPiece(sel_piece, selx, sely);
      }
      glutPostRedisplay();
      break;
    case GLUT_MIDDLE_BUTTON:
      middle_mouse = GL_TRUE;
      glutPostRedisplay();
      break;
    }
  } else {
    if (left_mouse) {
      left_mouse = GL_FALSE;
      dropSelection();
      glutPostRedisplay();
    } else if (middle_mouse) {
      middle_mouse = GL_FALSE;
      glutPostRedisplay();
    }
  }
  motion(x, y);
}

void
animate(void)
{
  if (spinning) {
    add_quats(lastquat, curquat, curquat);
  }
  glutPostRedisplay();
  if (solving) {
    if (!continueSolving()) {
      solving = 0;
      glutChangeToMenuEntry(1, (char *)"Solving", 1);
      glutSetWindowTitle((char *)"glpuzzle");
    }
  }
  if ((!solving && !spinning) || !visible) {
    glutIdleFunc(NULL);
  }
}

void
changeState(void)
{
  if (visible) {
    if (!solving && !spinning) {
      glutIdleFunc(NULL);
    } else {
      glutIdleFunc(animate);
    }
  } else {
    glutIdleFunc(NULL);
  }
}

void
init(void)
{
  static float lmodel_ambient[] =
  {0.0, 0.0, 0.0, 0.0};
  static float lmodel_twoside[] =
  {GL_FALSE};
  static float lmodel_local[] =
  {GL_FALSE};
  static float light0_ambient[] =
  {0.1f, 0.1f, 0.1f, 1.0f};
  static float light0_diffuse[] =
  {1.0f, 1.0f, 1.0f, 0.0f};
  static float light0_position[] =
  {0.8660254f, 0.5f, 1, 0};
  static float light0_specular[] =
  {0.0, 0.0, 0.0, 0.0};
  static float bevel_mat_ambient[] =
  {0.0, 0.0, 0.0, 1.0};
  static float bevel_mat_shininess[] =
  {40.0};
  static float bevel_mat_specular[] =
  {0.0, 0.0, 0.0, 0.0};
  static float bevel_mat_diffuse[] =
  {1.0, 0.0, 0.0, 0.0};

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  glClearDepth(1.0);

  glClearColor(0.5, 0.5, 0.5, 0.0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
  glEnable(GL_LIGHT0);

  glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, lmodel_local);
  glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
  glEnable(GL_LIGHTING);

  glMaterialfv(GL_FRONT, GL_AMBIENT, bevel_mat_ambient);
  glMaterialfv(GL_FRONT, GL_SHININESS, bevel_mat_shininess);
  glMaterialfv(GL_FRONT, GL_SPECULAR, bevel_mat_specular);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, bevel_mat_diffuse);

  glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_FLAT);

  trackball(curquat, 0.0, 0.0, 0.0, 0.0);
  srandom((unsigned int)time(NULL));
}

static void
Usage(void)
{
  puts("Usage: puzzle [-s]");
  puts("   -s:  Run in single buffered mode");
  exit(-1);
}

void
visibility(int v)
{
  if (v == GLUT_VISIBLE) {
    visible = 1;
  } else {
    visible = 0;
  }
  changeState();
}

void
menu(int choice)
{
   switch(choice) {
   case 1:
      reset_position();
      break;
   case 2:
      toggleSolve();
      break;
   case 3:
      reset();
      break;
   case 4:
      exit(0);
      break;
   }
}

int
main(int argc, char **argv)
{
  long i;

  Fl::use_high_res_GL(1);
  glutInit(&argc, argv);
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'n':
        enable_spinning = 0; // disable (sometimes annoying) spinning behaviour
        break;
      case 's':
        doubleBuffer = 0;
        break;
      default:
        Usage();
      }
    } else {
      Usage();
    }
  }

  glutInitWindowSize((int)W, (int)H);
  if (doubleBuffer) {
    glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
  } else {
    glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_SINGLE | GLUT_MULTISAMPLE);
  }

  glutCreateWindow("glpuzzle");
  visible = 1; // added for fltk, bug in original program?

  init();

  glGetIntegerv(GL_VIEWPORT, viewport);

  puts("");
  puts("n   Normal position - stop spinning");
  puts("r   Reset puzzle");
  puts("s   Solve puzzle (may take a few seconds to compute)");
  puts("d   Destroy a piece - makes the puzzle easier");
  puts("b   Toggles the depth buffer on and off");
  puts("");
  puts("Left mouse moves pieces");
  puts("Middle mouse spins the puzzle");
  puts("Right mouse has menu");

  glutReshapeFunc(Reshape);
  glutDisplayFunc(redraw);
  glutKeyboardFunc(keyboard);
  glutMotionFunc(motion);
  glutMouseFunc(mouse);
  glutVisibilityFunc(visibility);
  glutCreateMenu(menu);
  glutAddMenuEntry("Normal pos", 1);
  glutAddMenuEntry("Solve", 2);
  glutAddMenuEntry("Reset", 3);
  glutAddMenuEntry("Quit", 4);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}

#endif // added for fltk's distribution
