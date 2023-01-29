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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <FL/Fl_Int_Vector.H>

#define UNASSIGNED 0

class Sudoku_Generator {
private:
public:
  int grid[9][9];
  int solnGrid[9][9];
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
  void genPuzzle();
  bool gridStatus();
  void calculateDifficulty();
  int branchDifficultyScore();
};

// Generate a random number between 0 and maxLimit-1
int genRandNum(int maxLimit)
{
  return rand()%maxLimit;
}


// We take an integer array of the length n and swap the content around randomly.
// The function was part of the c++11 standard, but was removed in C++17 because
// it was regarded as flawed.
// This implementation is a minimal hack without care about random dstribution.
// For additional randomness, we do that three times.
void random_shuffle(int *data, int n, int (*r)(int))
{
  for (int n = 3; n>0; --n) {
    for (int i = n-1; i > 0; --i) {
      int j = r(i+1);
      int tmp = data[i];
      data[i] = data[j];
      data[j] = tmp;
    }
  }
}

// Helper functions for solving grid
bool FindUnassignedLocation(int grid[9][9], int &row, int &col)
{
  for (row = 0; row < 9; row++)
  {
    for (col = 0; col < 9; col++)
    {
      if (grid[row][col] == UNASSIGNED)
        return true;
    }
  }
  return false;
}

// Return true if num exists in row of grid.
bool UsedInRow(int grid[9][9], int row, int num)
{
  for (int col = 0; col < 9; col++)
  {
    if (grid[row][col] == num)
      return true;
  }
  return false;
}

//  Return true if num exists in col of grid.
bool UsedInCol(int grid[9][9], int col, int num)
{
  for (int row = 0; row < 9; row++)
  {
    if (grid[row][col] == num)
      return true;
  }
  return false;
}

// Return true if num exists in box at row, col of grid.
bool UsedInBox(int grid[9][9], int boxStartRow, int boxStartCol, int num)
{
  for (int row = 0; row < 3; row++)
  {
    for (int col = 0; col < 3; col++)
    {
      if (grid[row+boxStartRow][col+boxStartCol] == num)
        return true;
    }
  }
  return false;
}

// Return true if a number can be used at row, col and does not appear
// yet in the rom, column, or box
bool isSafe(int grid[9][9], int row, int col, int num)
{
  return !UsedInRow(grid, row, num) && !UsedInCol(grid, col, num) && !UsedInBox(grid, row - row%3 , col - col%3, num);
}

// Fill the box at idx*3, idx*3 with random values
void Sudoku_Generator::fillEmptyDiagonalBox(int idx)
{
  int start = idx*3;
  random_shuffle(guessNum, 9, genRandNum);
  for (int i = 0; i < 3; ++i)
  {
    for (int j = 0; j < 3; ++j)
    {
      this->grid[start+i][start+j] = guessNum[i*3+j];
    }
  }
}

// Create a soved Sudoku puzzle
void Sudoku_Generator::createSeed()
{
  /* Fill diagonal boxes to form:
   x | . | .
   . | x | .
   . | . | x
   */
  this->fillEmptyDiagonalBox(0);
  this->fillEmptyDiagonalBox(1);
  this->fillEmptyDiagonalBox(2);

  /* Fill the remaining blocks:
   x | x | x
   x | x | x
   x | x | x
   */
  this->solveGrid(); // Not truly random, but still good enough because we generate random diagonals.

  // Saving the solution grid
  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      this->solnGrid[i][j] = this->grid[i][j];
    }
  }
}

// Initialize the egenrator
Sudoku_Generator::Sudoku_Generator()
{
  // initialize difficulty level
  this->difficultyLevel = 0;

  // Randomly shuffling the array of removing grid positions
  for(int i=0;i<81;i++)
  {
    this->gridPos[i] = i;
  }

  random_shuffle(gridPos, 81, genRandNum);

  // Randomly shuffling the guessing number array
  for(int i=0;i<9;i++)
  {
    this->guessNum[i]=i+1;
  }

  random_shuffle(guessNum, 9, genRandNum);

  // Initialising the grid
  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      this->grid[i][j]=0;
    }
  }

  grid_status = true;
}

// Custom Initialising with grid passed as argument
Sudoku_Generator::Sudoku_Generator(int grid_data[81], bool row_major)
{
  // First pass: Check if all cells are valid
  for(int i=0; i<81; ++i)
  {
    int curr_num = grid_data[i];
    if(!((curr_num == UNASSIGNED) || (curr_num > 0 && curr_num < 10)))
    {
      grid_status=false;
      return;
    }

    if(row_major) grid[i/9][i%9] = curr_num;
    else          grid[i%9][i/9] = curr_num;
  }

  // Second pass: Check if all columns are valid
  for (int col_num=0; col_num<9; ++col_num)
  {
    bool nums[10]={false};
    for (int row_num=0; row_num<9; ++row_num)
    {
      int curr_num = grid[row_num][col_num];
      if(curr_num!=UNASSIGNED && nums[curr_num]==true)
      {
        grid_status=false;
        return;
      }
      nums[curr_num] = true;
    }
  }

  // Third pass: Check if all rows are valid
  for (int row_num=0; row_num<9; ++row_num)
  {
    bool nums[10]={false};
    for (int col_num=0; col_num<9; ++col_num)
    {
      int curr_num = grid[row_num][col_num];
      if(curr_num!=UNASSIGNED && nums[curr_num]==true)
      {
        grid_status=false;
        return;
      }
      nums[curr_num] = true;
    }
  }

  // Fourth pass: Check if all blocks are valid
  for (int block_num=0; block_num<9; ++block_num)
  {
    bool nums[10]={false};
    for (int cell_num=0; cell_num<9; ++cell_num)
    {
      int curr_num = grid[((int)(block_num/3))*3 + (cell_num/3)][((int)(block_num%3))*3 + (cell_num%3)];
      if(curr_num!=UNASSIGNED && nums[curr_num]==true)
      {
        grid_status=false;
        return;
      }
      nums[curr_num] = true;
    }
  }

  // Randomly shuffling the guessing number array
  for(int i=0;i<9;i++)
  {
    this->guessNum[i]=i+1;
  }

  random_shuffle(guessNum, 9, genRandNum);

  grid_status = true;
}

// Return status of the custom grid passed.
bool Sudoku_Generator::gridStatus()
{
  return grid_status;
}

// Printing the grid
void Sudoku_Generator::printGrid()
{
  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      if(grid[i][j] == 0)
        printf(" .");
      else
        printf(" %d", grid[i][j]);
      if (((j%3) == 2) && (j < 8))
        printf(" |");
    }
    printf("\n");
    if (((i%3) == 2) && (i < 8))
      printf("-------+-------+-------\n");
  }
  printf("\nDifficulty of current sudoku(0 being easiest): %d\n", difficultyLevel);
}

// Modified Sudoku solver
bool Sudoku_Generator::solveGrid()
{
  int row, col;

  // If there is no unassigned location, we are done
  if (!FindUnassignedLocation(this->grid, row, col))
    return true; // success!

  // Consider digits 1 to 9
  for (int num = 0; num < 9; num++)
  {
    // if looks promising
    if (isSafe(this->grid, row, col, this->guessNum[num]))
    {
      // make tentative assignment
      this->grid[row][col] = this->guessNum[num];

      // return, if success, yay!
      if (solveGrid())
        return true;

      // failure, unmake & try again
      this->grid[row][col] = UNASSIGNED;
    }
  }

  return false; // this triggers backtracking
}

// Check if the grid is uniquely solvable
void Sudoku_Generator::countSoln(int &number)
{
  int row, col;

  if(!FindUnassignedLocation(this->grid, row, col))
  {
    number++;
    return ;
  }


  for(int i=0;i<9 && number<2;i++)
  {
    if( isSafe(this->grid, row, col, this->guessNum[i]) )
    {
      this->grid[row][col] = this->guessNum[i];
      countSoln(number);
    }

    this->grid[row][col] = UNASSIGNED;
  }

}
// END: Check if the grid is uniquely solvable


// START: Generate puzzle
void Sudoku_Generator::genPuzzle()
{
  for(int i=0;i<81;i++)
  {
    int x = (this->gridPos[i])/9;
    int y = (this->gridPos[i])%9;
    int temp = this->grid[x][y];
    this->grid[x][y] = UNASSIGNED;

    // If now more than 1 solution , replace the removed cell back.
    int check=0;
    countSoln(check);
    if(check!=1)
    {
      this->grid[x][y] = temp;
    }
  }
}
// END: Generate puzzle


// START: Calculate branch difficulty score
int Sudoku_Generator::branchDifficultyScore()
{
  int emptyPositions = -1;
  int tempGrid[9][9];
  int sum=0;

  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      tempGrid[i][j] = this->grid[i][j];
    }
  }

  while(emptyPositions!=0)
  {
    Fl_Int_Vector empty[81];
    int empty_n = 0;

    for(int i=0;i<81;i++)
    {
      if(tempGrid[(int)(i/9)][(int)(i%9)] == 0)
      {
        // TODO: C++
        Fl_Int_Vector temp;
        temp.push_back(i);

        for(int num=1;num<=9;num++)
        {
          if(isSafe(tempGrid,i/9,i%9,num))
          {
            temp.push_back(num);
          }
        }

        empty[empty_n++] = temp;
      }

    }

    if(empty_n == 0)
    {
      return sum;
    }

    int minIndex = 0;

    int check = empty_n;
    for(int i=0;i<check;i++)
    {
      if(empty[i].size() < empty[minIndex].size())
        minIndex = i;
    }

    int branchFactor=empty[minIndex].size();
    int rowIndex = empty[minIndex][0]/9;
    int colIndex = empty[minIndex][0]%9;

    tempGrid[rowIndex][colIndex] = this->solnGrid[rowIndex][colIndex];
    sum = sum + ((branchFactor-2) * (branchFactor-2)) ;

    emptyPositions = empty_n - 1;
  }

  return sum;

}
// END: Finish branch difficulty score


// START: Calculate difficulty level of current grid
void Sudoku_Generator::calculateDifficulty()
{
  int B = branchDifficultyScore();
  int emptyCells = 0;

  for(int i=0;i<9;i++)
  {
    for(int j=0;j<9;j++)
    {
      if(this->grid[i][j] == 0)
        emptyCells++;
    }
  }

  this->difficultyLevel = B*100 + emptyCells;
}
// END: calculating difficulty level


// START: The main function
int generate_sudoku(int grid_data[81])
{
#if 0
  int i, j;
  FILE *f = fopen("/Users/matt/dev/su.cxx", "wb");
  fprintf(f, "// all horizontal chains\n");
  for (i=0; i<9; i++) {
    fprintf(f, "{ ");
    for (j=0; j<9; j++) {
      fprintf(f, "%2d, ", i*9+j);
    }
    fprintf(f, "},\n");
  }
  fprintf(f, "// all vertical chains\n");
  for (i=0; i<9; i++) {
    fprintf(f, "{ ");
    for (j=0; j<9; j++) {
      fprintf(f, "%2d, ", j*9+i);
    }
    fprintf(f, "},\n");
  }
  fprintf(f, "// all squares\n");
  for (i=0; i<9; i++) {
    fprintf(f, "{ ");
    for (j=0; j<9; j++) {
      fprintf(f, "%2d, ", ((i%3)*3) + ((i/3)*3*9) + (j%3) + (j/3)*9);
    }
    fprintf(f, "},\n");
  }
  fprintf(f, "// every field is part of 3 chains\n");
  for (i=0; i<81; i++) {
    fprintf(f, "{ ");
    int col = i % 9;
    int row = i / 9;
    fprintf(f, " %2d, %2d, %2d ",
           i/9, i%9, (col/3) + (row/3)*3
           );
    fprintf(f, "},\n");
  }
  fclose(f);
#endif


  // Initialising seed for random number generation
  srand((unsigned int)time(NULL));

  // Creating an instance of Sudoku
  Sudoku_Generator *puzzle = new Sudoku_Generator();

  // Creating a seed for puzzle generation
  puzzle->createSeed();

  // Generating the puzzle
  puzzle->genPuzzle();

  // Calculating difficulty of puzzle
  puzzle->calculateDifficulty();

  // testing by printing the grid
  puzzle->printGrid();

//  // Printing the grid into SVG file
//  string rem = "sudokuGen";
//  string path = argv[0];
//  path = path.substr(0,path.size() - rem.size());
//  puzzle->printSVG(path);
//  cout<<"The above sudoku puzzle has been stored in puzzles.svg in current folder\n";
//  // freeing the memory

  puzzle->printGrid();
  printf("Difficulty: %d\n", puzzle->difficultyLevel);
  for (int d = 0; d<9; d++) {
    int x = 0, y = 0;
    for (;;) {
      x = genRandNum(9);
      y = genRandNum(9);
      if (puzzle->grid[x][y] == 0) break;
    }
    puzzle->grid[x][y] = puzzle->solnGrid[x][y];
    printf(" %d %d\n", x, y);
    puzzle->calculateDifficulty();
    printf("Difficulty: %d\n", puzzle->difficultyLevel);
  }

  int *g = grid_data;
  for(int i=0;i<9;i++) {
    for(int j=0;j<9;j++) {
      if (puzzle->grid[i][j] == UNASSIGNED) {
        *g++ = -puzzle->solnGrid[i][j];
      } else {
        *g++ = puzzle->solnGrid[i][j];
      }
    }
  }

  delete puzzle;

  return 0;
}
// END: The main function
