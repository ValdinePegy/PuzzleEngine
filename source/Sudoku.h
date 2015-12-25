//  This file is part of PuzzleEngine, https://github.com/mercere99/PuzzleEngine/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This class defines a single sudoku puzzle instance.
//
//  For the moment, we are going to assume that the boards are all 9x9 with a
//  standard Sudoku layout.  In the future, we should be able to make the board
//  configuration much more flexible.

#ifndef PZE_SUDOKU_H
#define PZE_SUDOKU_H

#include <array>
#include <fstream>
#include <istream>
#include <set>
#include <vector>
#include "tools/assert.h"
#include "tools/Random.h"
#include "tools/string_utils.h"
#include "Puzzle.h"

namespace pze {
  class SudokuMove : public PuzzleMove {
  private:
    int cell_id;
    int state;

  public:
    SudokuMove() = default;
    ~SudokuMove() { ; }

    int GetID() const override { return cell_id; }
    int GetState() const override { return state; }
  };


  class Sudoku;

  class SudokuState : public PuzzleState {
  private:
    std::array<char,81> value;                   // Known value for cells; -1 = unknown.
    std::array<char,81> opt_count;               // How many options does each cell have?
    std::array<std::array<bool,9>, 81> options;  // Which options are available to each cell?
    const Sudoku * puzzle;                       // Pointer back to original puzzle.
    
    // "members" tracks which cell ids are members of each region.
    static constexpr int members[27][9] = {
      // Rows
      {  0,  1,  2,  3,  4,  5,  6,  7,  8 },  // Region 0
      {  9, 10, 11, 12, 13, 14, 15, 16, 17 },  // Region 1
      { 18, 19, 20, 21, 22, 23, 24, 25, 26 },  // Region 2
      { 27, 28, 29, 30, 31, 32, 33, 34, 35 },  // Region 3
      { 36, 37, 38, 39, 40, 41, 42, 43, 44 },  // Region 4
      { 45, 46, 47, 48, 49, 50, 51, 52, 53 },  // Region 5
      { 54, 55, 56, 57, 58, 59, 60, 61, 62 },  // Region 6
      { 63, 64, 65, 66, 67, 68, 69, 70, 71 },  // Region 7
      { 72, 73, 74, 75, 76, 77, 78, 79, 80 },  // Region 8

      // Columns
      { 0,  9, 18, 27, 36, 45, 54, 63, 72 },   // Region 9
      { 1, 10, 19, 28, 37, 46, 55, 64, 73 },   // Region 10
      { 2, 11, 20, 29, 38, 47, 56, 65, 74 },   // Region 11
      { 3, 12, 21, 30, 39, 48, 57, 66, 75 },   // Region 12
      { 4, 13, 22, 31, 40, 49, 58, 67, 76 },   // Region 13
      { 5, 14, 23, 32, 41, 50, 59, 68, 77 },   // Region 14
      { 6, 15, 24, 33, 42, 51, 60, 69, 78 },   // Region 15
      { 7, 16, 25, 34, 43, 52, 61, 70, 79 },   // Region 16
      { 8, 17, 26, 35, 44, 53, 62, 71, 80 },   // Region 17

      // Box Regions
      {  0,  1,  2,  9, 10, 11, 18, 19, 20 },  // Region 18
      {  3,  4,  5, 12, 13, 14, 21, 22, 23 },  // Region 19
      {  6,  7,  8, 15, 16, 17, 24, 25, 26 },  // Region 20
      { 27, 28, 29, 36, 37, 38, 45, 46, 47 },  // Region 21
      { 30, 31, 32, 39, 40, 41, 48, 49, 50 },  // Region 22
      { 33, 34, 35, 42, 43, 44, 51, 52, 53 },  // Region 23
      { 54, 55, 56, 63, 64, 65, 72, 73, 74 },  // Region 24
      { 57, 58, 59, 66, 67, 68, 75, 76, 77 },  // Region 25
      { 60, 61, 62, 69, 70, 71, 78, 79, 80 }   // Region 26
    };
    
    // "regions" tracks which regions each cell is a member of.
    static constexpr int regions[81][3] = {
      // { ROW, COLUMN, BOX }
      { 0,  9, 18 },  { 0, 10, 18 }, { 0, 11, 18 },  // Cells  0- 2
      { 0, 12, 19 },  { 0, 13, 19 }, { 0, 14, 19 },  // Cells  3- 5
      { 0, 15, 20 },  { 0, 16, 20 }, { 0, 17, 20 },  // Cells  6- 8
      { 1,  9, 18 },  { 1, 10, 18 }, { 1, 11, 18 },  // Cells  9-11
      { 1, 12, 19 },  { 1, 13, 19 }, { 1, 14, 19 },  // Cells 12-14
      { 1, 15, 20 },  { 1, 16, 20 }, { 1, 17, 20 },  // Cells 15-17
      { 2,  9, 18 },  { 2, 10, 18 }, { 2, 11, 18 },  // Cells 18-20
      { 2, 12, 19 },  { 2, 13, 19 }, { 2, 14, 19 },  // Cells 21-23
      { 2, 15, 20 },  { 2, 16, 20 }, { 2, 17, 20 },  // Cells 24-26
      
      { 3,  9, 21 },  { 3, 10, 21 }, { 3, 11, 21 },  // Cells 27-29
      { 3, 12, 22 },  { 3, 13, 22 }, { 3, 14, 22 },  // Cells 30-32
      { 3, 15, 23 },  { 3, 16, 23 }, { 3, 17, 23 },  // Cells 33-35
      { 4,  9, 21 },  { 4, 10, 21 }, { 4, 11, 21 },  // Cells 36-38
      { 4, 12, 22 },  { 4, 13, 22 }, { 4, 14, 22 },  // Cells 39-41
      { 4, 15, 23 },  { 4, 16, 23 }, { 4, 17, 23 },  // Cells 42-44
      { 5,  9, 21 },  { 5, 10, 21 }, { 5, 11, 21 },  // Cells 45-47
      { 5, 12, 22 },  { 5, 13, 22 }, { 5, 14, 22 },  // Cells 48-50
      { 5, 15, 23 },  { 5, 16, 23 }, { 5, 17, 23 },  // Cells 51-53
      
      { 6,  9, 24 },  { 6, 10, 24 }, { 6, 11, 24 },  // Cells 54-56
      { 6, 12, 25 },  { 6, 13, 25 }, { 6, 14, 25 },  // Cells 57-59
      { 6, 15, 26 },  { 6, 16, 26 }, { 6, 17, 26 },  // Cells 60-62
      { 7,  9, 24 },  { 7, 10, 24 }, { 7, 11, 24 },  // Cells 63-65
      { 7, 12, 25 },  { 7, 13, 25 }, { 7, 14, 25 },  // Cells 66-68
      { 7, 15, 26 },  { 7, 16, 26 }, { 7, 17, 26 },  // Cells 69-71
      { 8,  9, 24 },  { 8, 10, 24 }, { 8, 11, 24 },  // Cells 72-74
      { 8, 12, 25 },  { 8, 13, 25 }, { 8, 14, 25 },  // Cells 75-77
      { 8, 15, 26 },  { 8, 16, 26 }, { 8, 17, 26 }   // Cells 78-80
    };

    static constexpr int links[81][20] = {
      { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 18, 19, 20, 27, 36, 45, 54, 63, 72 },
      { 0,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 18, 19, 20, 28, 37, 46, 55, 64, 73 },
      { 0,  1,  3,  4,  5,  6,  7,  8,  9, 10, 11, 18, 19, 20, 29, 38, 47, 56, 65, 74 },
      { 0,  1,  2,  4,  5,  6,  7,  8, 12, 13, 14, 21, 22, 23, 30, 39, 48, 57, 66, 75 },
      { 0,  1,  2,  3,  5,  6,  7,  8, 12, 13, 14, 21, 22, 23, 31, 40, 49, 58, 67, 76 },
      { 0,  1,  2,  3,  4,  6,  7,  8, 12, 13, 14, 21, 22, 23, 32, 41, 50, 59, 68, 77 },
      { 0,  1,  2,  3,  4,  5,  7,  8, 15, 16, 17, 24, 25, 26, 33, 42, 51, 60, 69, 78 },
      { 0,  1,  2,  3,  4,  5,  6,  8, 15, 16, 17, 24, 25, 26, 34, 43, 52, 61, 70, 79 },
      { 0,  1,  2,  3,  4,  5,  6,  7, 15, 16, 17, 24, 25, 26, 35, 44, 53, 62, 71, 80 },
      { 0,  1,  2, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 27, 36, 45, 54, 63, 72 },
      { 0,  1,  2,  9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 28, 37, 46, 55, 64, 73 },
      { 0,  1,  2,  9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 29, 38, 47, 56, 65, 74 },
      { 3,  4,  5,  9, 10, 11, 13, 14, 15, 16, 17, 21, 22, 23, 30, 39, 48, 57, 66, 75 },
      { 3,  4,  5,  9, 10, 11, 12, 14, 15, 16, 17, 21, 22, 23, 31, 40, 49, 58, 67, 76 },
      { 3,  4,  5,  9, 10, 11, 12, 13, 15, 16, 17, 21, 22, 23, 32, 41, 50, 59, 68, 77 },
      { 6,  7,  8,  9, 10, 11, 12, 13, 14, 16, 17, 24, 25, 26, 33, 42, 51, 60, 69, 78 },
      { 6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 17, 24, 25, 26, 34, 43, 52, 61, 70, 79 },
      { 6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 24, 25, 26, 35, 44, 53, 62, 71, 80 },
      { 0,  1,  2,  9, 10, 11, 19, 20, 21, 22, 23, 24, 25, 26, 27, 36, 45, 54, 63, 72 },
      { 0,  1,  2,  9, 10, 11, 18, 20, 21, 22, 23, 24, 25, 26, 28, 37, 46, 55, 64, 73 },
      { 0,  1,  2,  9, 10, 11, 18, 19, 21, 22, 23, 24, 25, 26, 29, 38, 47, 56, 65, 74 },
      { 3,  4,  5, 12, 13, 14, 18, 19, 20, 22, 23, 24, 25, 26, 30, 39, 48, 57, 66, 75 },
      { 3,  4,  5, 12, 13, 14, 18, 19, 20, 21, 23, 24, 25, 26, 31, 40, 49, 58, 67, 76 },
      { 3,  4,  5, 12, 13, 14, 18, 19, 20, 21, 22, 24, 25, 26, 32, 41, 50, 59, 68, 77 },
      { 6,  7,  8, 15, 16, 17, 18, 19, 20, 21, 22, 23, 25, 26, 33, 42, 51, 60, 69, 78 },
      { 6,  7,  8, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 26, 34, 43, 52, 61, 70, 79 },
      { 6,  7,  8, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 35, 44, 53, 62, 71, 80 },
      { 0,  9, 18, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 45, 46, 47, 54, 63, 72 },
      { 1, 10, 19, 27, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 45, 46, 47, 55, 64, 73 },
      { 2, 11, 20, 27, 28, 30, 31, 32, 33, 34, 35, 36, 37, 38, 45, 46, 47, 56, 65, 74 },
      { 3, 12, 21, 27, 28, 29, 31, 32, 33, 34, 35, 39, 40, 41, 48, 49, 50, 57, 66, 75 },
      { 4, 13, 22, 27, 28, 29, 30, 32, 33, 34, 35, 39, 40, 41, 48, 49, 50, 58, 67, 76 },
      { 5, 14, 23, 27, 28, 29, 30, 31, 33, 34, 35, 39, 40, 41, 48, 49, 50, 59, 68, 77 },
      { 6, 15, 24, 27, 28, 29, 30, 31, 32, 34, 35, 42, 43, 44, 51, 52, 53, 60, 69, 78 },
      { 7, 16, 25, 27, 28, 29, 30, 31, 32, 33, 35, 42, 43, 44, 51, 52, 53, 61, 70, 79 },
      { 8, 17, 26, 27, 28, 29, 30, 31, 32, 33, 34, 42, 43, 44, 51, 52, 53, 62, 71, 80 },
      { 0,  9, 18, 27, 28, 29, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 54, 63, 72 },
      { 1, 10, 19, 27, 28, 29, 36, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 55, 64, 73 },
      { 2, 11, 20, 27, 28, 29, 36, 37, 39, 40, 41, 42, 43, 44, 45, 46, 47, 56, 65, 74 },
      { 3, 12, 21, 30, 31, 32, 36, 37, 38, 40, 41, 42, 43, 44, 48, 49, 50, 57, 66, 75 },
      { 4, 13, 22, 30, 31, 32, 36, 37, 38, 39, 41, 42, 43, 44, 48, 49, 50, 58, 67, 76 },
      { 5, 14, 23, 30, 31, 32, 36, 37, 38, 39, 40, 42, 43, 44, 48, 49, 50, 59, 68, 77 },
      { 6, 15, 24, 33, 34, 35, 36, 37, 38, 39, 40, 41, 43, 44, 51, 52, 53, 60, 69, 78 },
      { 7, 16, 25, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 44, 51, 52, 53, 61, 70, 79 },
      { 8, 17, 26, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 51, 52, 53, 62, 71, 80 },
      { 0,  9, 18, 27, 28, 29, 36, 37, 38, 46, 47, 48, 49, 50, 51, 52, 53, 54, 63, 72 },
      { 1, 10, 19, 27, 28, 29, 36, 37, 38, 45, 47, 48, 49, 50, 51, 52, 53, 55, 64, 73 },
      { 2, 11, 20, 27, 28, 29, 36, 37, 38, 45, 46, 48, 49, 50, 51, 52, 53, 56, 65, 74 },
      { 3, 12, 21, 30, 31, 32, 39, 40, 41, 45, 46, 47, 49, 50, 51, 52, 53, 57, 66, 75 },
      { 4, 13, 22, 30, 31, 32, 39, 40, 41, 45, 46, 47, 48, 50, 51, 52, 53, 58, 67, 76 },
      { 5, 14, 23, 30, 31, 32, 39, 40, 41, 45, 46, 47, 48, 49, 51, 52, 53, 59, 68, 77 },
      { 6, 15, 24, 33, 34, 35, 42, 43, 44, 45, 46, 47, 48, 49, 50, 52, 53, 60, 69, 78 },
      { 7, 16, 25, 33, 34, 35, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 53, 61, 70, 79 },
      { 8, 17, 26, 33, 34, 35, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 62, 71, 80 },
      { 0,  9, 18, 27, 36, 45, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 72, 73, 74 },
      { 1, 10, 19, 28, 37, 46, 54, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 72, 73, 74 },
      { 2, 11, 20, 29, 38, 47, 54, 55, 57, 58, 59, 60, 61, 62, 63, 64, 65, 72, 73, 74 },
      { 3, 12, 21, 30, 39, 48, 54, 55, 56, 58, 59, 60, 61, 62, 66, 67, 68, 75, 76, 77 },
      { 4, 13, 22, 31, 40, 49, 54, 55, 56, 57, 59, 60, 61, 62, 66, 67, 68, 75, 76, 77 },
      { 5, 14, 23, 32, 41, 50, 54, 55, 56, 57, 58, 60, 61, 62, 66, 67, 68, 75, 76, 77 },
      { 6, 15, 24, 33, 42, 51, 54, 55, 56, 57, 58, 59, 61, 62, 69, 70, 71, 78, 79, 80 },
      { 7, 16, 25, 34, 43, 52, 54, 55, 56, 57, 58, 59, 60, 62, 69, 70, 71, 78, 79, 80 },
      { 8, 17, 26, 35, 44, 53, 54, 55, 56, 57, 58, 59, 60, 61, 69, 70, 71, 78, 79, 80 },
      { 0,  9, 18, 27, 36, 45, 54, 55, 56, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74 },
      { 1, 10, 19, 28, 37, 46, 54, 55, 56, 63, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74 },
      { 2, 11, 20, 29, 38, 47, 54, 55, 56, 63, 64, 66, 67, 68, 69, 70, 71, 72, 73, 74 },
      { 3, 12, 21, 30, 39, 48, 57, 58, 59, 63, 64, 65, 67, 68, 69, 70, 71, 75, 76, 77 },
      { 4, 13, 22, 31, 40, 49, 57, 58, 59, 63, 64, 65, 66, 68, 69, 70, 71, 75, 76, 77 },
      { 5, 14, 23, 32, 41, 50, 57, 58, 59, 63, 64, 65, 66, 67, 69, 70, 71, 75, 76, 77 },
      { 6, 15, 24, 33, 42, 51, 60, 61, 62, 63, 64, 65, 66, 67, 68, 70, 71, 78, 79, 80 },
      { 7, 16, 25, 34, 43, 52, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 71, 78, 79, 80 },
      { 8, 17, 26, 35, 44, 53, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 78, 79, 80 },
      { 0,  9, 18, 27, 36, 45, 54, 55, 56, 63, 64, 65, 73, 74, 75, 76, 77, 78, 79, 80 },
      { 1, 10, 19, 28, 37, 46, 54, 55, 56, 63, 64, 65, 72, 74, 75, 76, 77, 78, 79, 80 },
      { 2, 11, 20, 29, 38, 47, 54, 55, 56, 63, 64, 65, 72, 73, 75, 76, 77, 78, 79, 80 },
      { 3, 12, 21, 30, 39, 48, 57, 58, 59, 66, 67, 68, 72, 73, 74, 76, 77, 78, 79, 80 },
      { 4, 13, 22, 31, 40, 49, 57, 58, 59, 66, 67, 68, 72, 73, 74, 75, 77, 78, 79, 80 },
      { 5, 14, 23, 32, 41, 50, 57, 58, 59, 66, 67, 68, 72, 73, 74, 75, 76, 78, 79, 80 },
      { 6, 15, 24, 33, 42, 51, 60, 61, 62, 69, 70, 71, 72, 73, 74, 75, 76, 77, 79, 80 },
      { 7, 16, 25, 34, 43, 52, 60, 61, 62, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 80 },
      { 8, 17, 26, 35, 44, 53, 60, 61, 62, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79 }
    };
    
  public:
    SudokuState(Sudoku * p) : puzzle(p) { Clear(); }
    SudokuState(Sudoku & p) : puzzle(&p) { Clear(); }
    SudokuState(const SudokuState &) = default;
    ~SudokuState() { ; }

    SudokuState& operator=(const SudokuState &) = default;
    
    // A method to clear out all of the solution info when starting a new solve attempt.
    void Clear() override;

    // Set an individual cell in the current state; remove option from linked cells.
    bool Set(int cell, int state) override;

    // Remove a symbol option from a particular cell.
    bool Block(int cell, int state) override;

    // Operate on a "move" object.
    bool Move(const PuzzleMove & move) override;

    // Print the current state of the puzzle, including all options available.
    void Print(const std::array<char,9> & symbols, std::ostream & out=std::cout);
    void Print(std::ostream & out=std::cout) override;

    // Use a brute-force approach to completely solve this puzzle.
    // Return true if solved, false if unsolvable.
    bool ForceSolve(int start=0);
  };

  
  
  class Sudoku : public Puzzle {
  private:
    // Core puzzle info
    std::array<int,81> cells;         // What is the full solution?
    std::array<bool,81> start_cells;  // Is each cell visible at the start?
    std::array<char, 9> symbols;     // What symbols are used in this puzzle?
    
    // An iterative step to randomize the state of the grid.
    // Return whether a valid solution was involved.
    bool RandomizeCells_step(emp::Random & random, int next) {
      if (next == 81) return true;  // If we pass the end, we found a solution!

      // Test possible states for this cell in random order.
      std::vector<int> states( random.GetPermutation(9) );

      // @CAO ACTUALLY DO TESTS HERE!!
      
      // If all states have failed, return false (this is a dead-end)
      return false;
    }

  public:
    Sudoku() {
      cells = { 0,1,2, 3,4,5, 6,7,8, 
                5,7,4, 6,0,8, 1,2,3, 
                3,8,6, 1,7,2, 0,5,4, 
                8,2,0, 7,3,6, 4,1,5, 
                1,5,3, 8,2,4, 7,6,0, 
                6,4,7, 0,5,1, 3,8,2, 
                7,0,1, 5,8,3, 2,4,6, 
                4,6,5, 2,1,0, 8,3,7, 
                2,3,8, 4,6,7, 5,0,1
      };

      start_cells.fill(false);
    }
    Sudoku(const Sudoku &) = default;
    Sudoku(emp::Random & random, double start_prob=1.0) {
      RandomizeCells(random);
      RandomizeStart(random, start_prob);
    }
    Sudoku(std::istream & is) { Load(is); }
    Sudoku(const std::string & filename) { Load(filename); }
    
    ~Sudoku() { ; }

    const std::array<int,81> & GetCells() const { return cells; }
    const std::array<bool,81> & GtStartCells() const { return start_cells; }
    const std::array<char,9> & GetSymbols() const { return symbols; }
    SudokuState GetState();
    
    void SetStart(int id, bool start_ok=true) { start_cells[id] = start_ok; }

    bool Load(std::istream & is);

    bool Load(const std::string & filename) {
      std::ifstream f(filename);
      return Load(f);
    }
    
    void RandomizeCells(emp::Random & random);

    // Shuffle will reorganize the board changing symbols and row/column order, but
    // otherwise keep the core nature of the puzzle the same.
    void Shuffle(emp::Random & random);

    void RandomizeStart(emp::Random & random, double start_prob=1.0);

    void Print(std::ostream & out=std::cout) override;
  };
}

#endif
