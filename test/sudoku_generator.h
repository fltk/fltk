//
// Sudoku game generator using the Fast Light Tool Kit (FLTK).
//
// Copyright (c) 2018 Vaibhav Thakkar.
// Copyright 2023 by Vaibhav Thakkar and Matthias Melcher.
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

//
// This solver is based on the work of Vaibhav Thakkar
// from https://github.com/vaithak/Sudoku-Generator
// vaithak/Sudoku-Generator is licensed under the MIT License
// Copyright (c) 2018 Vaibhav Thakkar
//
// The solver was modified to fit FLTKs requirements of using minimal C++
// and adapted to the FLTK naming scheme.
//

#ifndef _SUDOKU_GENERATOR_H_
#define _SUDOKU_GENERATOR_H_

typedef int GridData[9][9];

extern int generate_sudoku(int grid_data[81], int minHints, int maxHints);

class Sudoku_Generator {
private:
public:
  GridData grid;
  GridData solnGrid;
  int guessNum[9];
  int gridPos[81];
  int difficultyLevel;
  bool grid_status;

public:
  Sudoku_Generator ();
  Sudoku_Generator(int[81], bool row_major=true);
  void fillEmptyDiagonalBox(int);
  void createSeed();
  void printGrid();
  bool solveGrid();
  void countSoln(int &number);
  void genPuzzle(int minHints);
  bool gridStatus();
  void calculateDifficulty();
  void restoreWorkGrid();
  int branchDifficultyScore();
};

#endif // _SUDOKU_GENERATOR_H_
