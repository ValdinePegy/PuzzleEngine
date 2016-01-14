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

  class Sudoku;

  class SudokuState : public PuzzleState {
  private:
    static constexpr int NUM_STATES = 9;
    static constexpr int NUM_ROWS = 9;
    static constexpr int NUM_COLS = 9;
    static constexpr int NUM_SQUARES = 9;
    static constexpr int NUM_OVERLAPS = 54;
    static constexpr int NUM_CELLS = NUM_ROWS * NUM_COLS;                  // 81
    static constexpr int NUM_REGIONS = NUM_ROWS + NUM_COLS + NUM_SQUARES;  // 27
    

    std::array<char,NUM_CELLS> value;         // Known value for cells; -1 = unknown
    std::array<uint32_t, NUM_CELLS> options;  // Options still available to each cell
    const Sudoku * puzzle;                    // Pointer back to original puzzle

    // "members" tracks which cell ids are members of each region.
    static constexpr int members[NUM_REGIONS][9] = {
      // Rows (Overlaps: 111000000, 000111000, 000000111)
      {  0,  1,  2,  3,  4,  5,  6,  7,  8 },  // Region 0  (Overlap with 18, 19, 20)
      {  9, 10, 11, 12, 13, 14, 15, 16, 17 },  // Region 1  (Overlap with 18, 19, 20)
      { 18, 19, 20, 21, 22, 23, 24, 25, 26 },  // Region 2  (Overlap with 18, 19, 20)
      { 27, 28, 29, 30, 31, 32, 33, 34, 35 },  // Region 3  (Overlap with 21, 22, 23)
      { 36, 37, 38, 39, 40, 41, 42, 43, 44 },  // Region 4  (Overlap with 21, 22, 23)
      { 45, 46, 47, 48, 49, 50, 51, 52, 53 },  // Region 5  (Overlap with 21, 22, 23)
      { 54, 55, 56, 57, 58, 59, 60, 61, 62 },  // Region 6  (Overlap with 24, 25, 26)
      { 63, 64, 65, 66, 67, 68, 69, 70, 71 },  // Region 7  (Overlap with 24, 25, 26)
      { 72, 73, 74, 75, 76, 77, 78, 79, 80 },  // Region 8  (Overlap with 24, 25, 26)

      // Columns (Overlaps: 111000000, 000111000, 000000111)
      { 0,  9, 18, 27, 36, 45, 54, 63, 72 },   // Region 9  (Overlap with 18, 21, 24)
      { 1, 10, 19, 28, 37, 46, 55, 64, 73 },   // Region 10 (Overlap with 18, 21, 24)
      { 2, 11, 20, 29, 38, 47, 56, 65, 74 },   // Region 11 (Overlap with 18, 21, 24)
      { 3, 12, 21, 30, 39, 48, 57, 66, 75 },   // Region 12 (Overlap with 19, 22, 25)
      { 4, 13, 22, 31, 40, 49, 58, 67, 76 },   // Region 13 (Overlap with 19, 22, 25)
      { 5, 14, 23, 32, 41, 50, 59, 68, 77 },   // Region 14 (Overlap with 19, 22, 25)
      { 6, 15, 24, 33, 42, 51, 60, 69, 78 },   // Region 15 (Overlap with 20, 23, 26)
      { 7, 16, 25, 34, 43, 52, 61, 70, 79 },   // Region 16 (Overlap with 20, 23, 26)
      { 8, 17, 26, 35, 44, 53, 62, 71, 80 },   // Region 17 (Overlap with 20, 23, 26)

      // Box Regions (Overlaps: 111000000, 000111000, 000000111, 100100100, 010010010, 001001001)
      {  0,  1,  2,  9, 10, 11, 18, 19, 20 },  // Region 18 (Overlap with 0, 1, 2,  9, 10, 11)
      {  3,  4,  5, 12, 13, 14, 21, 22, 23 },  // Region 19 (Overlap with 0, 1, 2, 12, 13, 14)
      {  6,  7,  8, 15, 16, 17, 24, 25, 26 },  // Region 20 (Overlap with 0, 1, 2, 15, 16, 17)
      { 27, 28, 29, 36, 37, 38, 45, 46, 47 },  // Region 21 (Overlap with 3, 4, 5,  9, 10, 11)
      { 30, 31, 32, 39, 40, 41, 48, 49, 50 },  // Region 22 (Overlap with 3, 4, 5, 12, 13, 14)
      { 33, 34, 35, 42, 43, 44, 51, 52, 53 },  // Region 23 (Overlap with 3, 4, 5, 15, 16, 17)
      { 54, 55, 56, 63, 64, 65, 72, 73, 74 },  // Region 24 (Overlap with 6, 7, 8,  9, 10, 11)
      { 57, 58, 59, 66, 67, 68, 75, 76, 77 },  // Region 25 (Overlap with 6, 7, 8, 12, 13, 14)
      { 60, 61, 62, 69, 70, 71, 78, 79, 80 }   // Region 26 (Overlap with 6, 7, 8, 15, 16, 17)
    };
    
    // "regions" tracks which regions each cell is a member of.
    static constexpr int regions[NUM_CELLS][3] = {
      // { ROW, COLUMN, BOX }
      { 0,  9, 18 }, { 0, 10, 18 }, { 0, 11, 18 },  // Cells  0- 2
      { 0, 12, 19 }, { 0, 13, 19 }, { 0, 14, 19 },  // Cells  3- 5
      { 0, 15, 20 }, { 0, 16, 20 }, { 0, 17, 20 },  // Cells  6- 8
      { 1,  9, 18 }, { 1, 10, 18 }, { 1, 11, 18 },  // Cells  9-11
      { 1, 12, 19 }, { 1, 13, 19 }, { 1, 14, 19 },  // Cells 12-14
      { 1, 15, 20 }, { 1, 16, 20 }, { 1, 17, 20 },  // Cells 15-17
      { 2,  9, 18 }, { 2, 10, 18 }, { 2, 11, 18 },  // Cells 18-20
      { 2, 12, 19 }, { 2, 13, 19 }, { 2, 14, 19 },  // Cells 21-23
      { 2, 15, 20 }, { 2, 16, 20 }, { 2, 17, 20 },  // Cells 24-26
      
      { 3,  9, 21 }, { 3, 10, 21 }, { 3, 11, 21 },  // Cells 27-29
      { 3, 12, 22 }, { 3, 13, 22 }, { 3, 14, 22 },  // Cells 30-32
      { 3, 15, 23 }, { 3, 16, 23 }, { 3, 17, 23 },  // Cells 33-35
      { 4,  9, 21 }, { 4, 10, 21 }, { 4, 11, 21 },  // Cells 36-38
      { 4, 12, 22 }, { 4, 13, 22 }, { 4, 14, 22 },  // Cells 39-41
      { 4, 15, 23 }, { 4, 16, 23 }, { 4, 17, 23 },  // Cells 42-44
      { 5,  9, 21 }, { 5, 10, 21 }, { 5, 11, 21 },  // Cells 45-47
      { 5, 12, 22 }, { 5, 13, 22 }, { 5, 14, 22 },  // Cells 48-50
      { 5, 15, 23 }, { 5, 16, 23 }, { 5, 17, 23 },  // Cells 51-53
      
      { 6,  9, 24 }, { 6, 10, 24 }, { 6, 11, 24 },  // Cells 54-56
      { 6, 12, 25 }, { 6, 13, 25 }, { 6, 14, 25 },  // Cells 57-59
      { 6, 15, 26 }, { 6, 16, 26 }, { 6, 17, 26 },  // Cells 60-62
      { 7,  9, 24 }, { 7, 10, 24 }, { 7, 11, 24 },  // Cells 63-65
      { 7, 12, 25 }, { 7, 13, 25 }, { 7, 14, 25 },  // Cells 66-68
      { 7, 15, 26 }, { 7, 16, 26 }, { 7, 17, 26 },  // Cells 69-71
      { 8,  9, 24 }, { 8, 10, 24 }, { 8, 11, 24 },  // Cells 72-74
      { 8, 12, 25 }, { 8, 13, 25 }, { 8, 14, 25 },  // Cells 75-77
      { 8, 15, 26 }, { 8, 16, 26 }, { 8, 17, 26 }   // Cells 78-80
    };

    // Which *other* cells is each cell linked to by at least one region?
    static constexpr int links[NUM_CELLS][20] = {
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
    
    // Given a binary representation of options, which bit position is the first available?
    static constexpr int next_opt[512] = {
      -1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
      4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    };

    // Given a binary representation of state options, how many are available?
    static constexpr int opts_count[512] = {
      0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
      5, 6, 6, 7, 6, 7, 7, 8, 6, 7, 7, 8, 7, 8, 8, 9
    };

    // Which cells are in region overlaps (horizontal or vertical)?
    static constexpr int overlaps[NUM_OVERLAPS][3] = {
      {  0,  1,  2 }, {  3,  4,  5 }, {  6,  7,  8 },  // Overlap sets  0,  1,  2 (Row 0)
      {  9, 10, 11 }, { 12, 13, 14 }, { 15, 16, 17 },  // Overlap sets  3,  4,  5 (Row 1)
      { 18, 19, 20 }, { 21, 22, 23 }, { 24, 25, 26 },  // Overlap sets  6,  7,  8 (Row 2)
      { 27, 28, 29 }, { 30, 31, 32 }, { 33, 34, 35 },  // Overlap sets  9, 10, 11 (Row 3)
      { 36, 37, 38 }, { 39, 40, 41 }, { 42, 43, 44 },  // Overlap sets 12, 13, 14 (Row 4)
      { 45, 46, 47 }, { 48, 49, 50 }, { 51, 52, 53 },  // Overlap sets 15, 16, 17 (Row 5)
      { 54, 55, 56 }, { 57, 58, 59 }, { 60, 61, 62 },  // Overlap sets 18, 19, 20 (Row 6)
      { 63, 64, 65 }, { 66, 67, 68 }, { 69, 70, 71 },  // Overlap sets 21, 22, 23 (Row 7)
      { 72, 73, 74 }, { 75, 76, 77 }, { 78, 79, 80 },  // Overlap sets 24, 25, 26 (Row 8)

      {  0,  9, 18 }, { 27, 36, 45 }, { 54, 63, 72 },  // Overlap sets 27, 28, 29 (Col 0)
      {  1, 10, 19 }, { 28, 37, 46 }, { 55, 64, 73 },  // Overlap sets 30, 31, 32 (Col 1)
      {  2, 11, 20 }, { 29, 38, 47 }, { 56, 65, 74 },  // Overlap sets 33, 34, 35 (Col 2)
      {  3, 12, 21 }, { 30, 39, 48 }, { 57, 66, 75 },  // Overlap sets 36, 37, 38 (Col 3)
      {  4, 13, 22 }, { 31, 40, 49 }, { 58, 67, 76 },  // Overlap sets 39, 40, 41 (Col 4)
      {  5, 14, 23 }, { 32, 41, 50 }, { 59, 68, 77 },  // Overlap sets 42, 43, 44 (Col 5)
      {  6, 15, 24 }, { 33, 42, 51 }, { 60, 69, 78 },  // Overlap sets 45, 46, 47 (Col 6)
      {  7, 16, 25 }, { 34, 43, 52 }, { 61, 70, 79 },  // Overlap sets 48, 49, 50 (Col 7)
      {  8, 17, 26 }, { 35, 44, 53 }, { 62, 71, 80 },  // Overlap sets 51, 52, 53 (Col 8)
    };

    // Which overlaps are square regions made up from? (each square is listed twice).
    static constexpr int square_overlaps[18][3] = {
      {  0,  3,  6 }, {  1,  4,  7 }, {  2,  5,  8 },
      {  9, 12, 15 }, { 10, 13, 16 }, { 11, 14, 17 },
      { 18, 21, 24 }, { 19, 22, 25 }, { 20, 23, 26 },

      { 27, 30, 33 }, { 28, 31, 34 }, { 29, 32, 35 },
      { 36, 39, 42 }, { 37, 40, 43 }, { 38, 41, 44 },
      { 45, 48, 51 }, { 46, 49, 52 }, { 47, 50, 53 }
    };

    // Which region is each overlap associated with? (row/col first, then square)
    static constexpr int overlap_regions[NUM_OVERLAPS][2] = {
      {  0, 18 }, {  0, 19 }, {  0, 20 },
      {  1, 18 }, {  1, 19 }, {  1, 20 },
      {  2, 18 }, {  2, 19 }, {  2, 20 },
      {  3, 21 }, {  3, 22 }, {  3, 23 },
      {  4, 21 }, {  4, 22 }, {  4, 23 },
      {  5, 21 }, {  5, 22 }, {  5, 23 },
      {  6, 24 }, {  6, 25 }, {  6, 26 },
      {  7, 24 }, {  7, 25 }, {  7, 26 },
      {  8, 24 }, {  8, 25 }, {  8, 26 },

      {  9, 18 }, {  9, 21 }, {  9, 24 },
      { 10, 18 }, { 10, 21 }, { 10, 24 },
      { 11, 18 }, { 11, 21 }, { 11, 24 },
      { 12, 19 }, { 12, 22 }, { 12, 25 },
      { 13, 19 }, { 13, 22 }, { 13, 25 },
      { 14, 19 }, { 14, 22 }, { 14, 25 },
      { 15, 20 }, { 15, 23 }, { 15, 26 },
      { 16, 20 }, { 16, 23 }, { 16, 26 },
      { 17, 20 }, { 17, 23 }, { 17, 26 }
    };
    
  public:
    SudokuState(const Sudoku * p) : puzzle(p) { Clear(); }
    SudokuState(const Sudoku & p) : puzzle(&p) { Clear(); }
    SudokuState(const SudokuState &) = default;
    ~SudokuState() { ; }

    SudokuState& operator=(const SudokuState &) = default;

    int GetValue(int cell) const { return value[cell]; }
    uint32_t GetOptions(int cell) const { return options[cell]; }
    int CountOptions(int cell) const {
      // if (cell < 0 || cell >= 81) std::cout << "cell=" << cell << std::endl;
      emp_assert(cell >= 0 && cell < 81, cell);
      return opts_count[options[cell]];
    }
    const Sudoku * const GetPuzzle() const { return puzzle; }
    bool HasOption(int cell, int state) {
      emp_assert(cell >= 0 && cell < 81, cell);
      emp_assert(state >= 0 && state < 9, state);
      return options[cell] & (1 << state);
    }
    bool IsSet(int cell) const { return value[cell] != -1; }
    bool IsSolved() {
      for (uint32_t o : options) if (o) return false;
      return true;
    }
    
    // A method to clear out all of the solution info when starting a new solve attempt.
    void Clear() override;

    // Find the next available option for a cell.
    int FindNext(int cell) { return next_opt[options[cell]]; }

    // Set the value of an individual cell; remove option from linked cells.
    // Return true/false based on whether progress was made toward solving the puzzle.
    void Set(int cell, int value) override;
    
    // Remove a symbol option from a particular cell.
    void Block(int cell, int state) override { options[cell] &= ~(1 << state); }

    // Operate on a "move" object.
    void Move(const PuzzleMove & move) override;
    using PuzzleState::Move;
    
    // Print the current state of the puzzle, including all options available.
    void Print(const std::array<char,9> & symbols, std::ostream & out=std::cout);
    void Print(std::ostream & out=std::cout) override;

    // Use a brute-force approach to completely solve this puzzle.
    // Return true if solved, false if unsolvable.
    bool ForceSolve(int start=0);

    
    // More human-focused solving techniques:

    // If there's only one state a cell can be, pick it!
    std::vector<PuzzleMove> Solve_FindLastCellState();

    // If there's only one cell that can have a certain state in a region, choose it!
    std::vector<PuzzleMove> Solve_FindLastRegionState();

    // If only cells that can have a state in region A are all also in region
    // B, no other cell in region B can have that state as a possibility.
    std::vector<PuzzleMove> Solve_FindRegionOverlap();
    
    // If K cells are all limited to the same K states, eliminate those states
    // from all other cells in the same region.
    std::vector<PuzzleMove> Solve_FindLimitedCells();
    
    // Eliminate all other possibilities from K cells if they are the only
    // ones that can possess K states in a single region.
    std::vector<PuzzleMove> Solve_FindLimitedStates();

    // If there are X rows (cols) where a certain state can only be in one of 
    // X cols (rows), then no other row in this cols can be that state.
    std::vector<PuzzleMove> Solve_FindSwordfish();


    // Make sure the current state is consistant.
    bool OK();
  };

  
  
  class Sudoku : public Puzzle {
  private:
    // Core puzzle info
    std::array<int,81> cells;         // What is the full solution?
    std::array<bool,81> start_cells;  // Is each cell visible at the start?
    std::array<char, 9> symbols;      // What symbols are used in this puzzle?
    mutable SudokuState start_state;  // Starting state for puzzle (init when needed)
    mutable bool init;                // Has this puzzle been initialized yet?
    
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

    void InitStartState() const {
      emp_assert(init == false);  // Make sure this state hasn't been initialized yet.

      // Set the proper cells in the start state
      start_state.Clear();
      for (int i = 0; i < 81; i++) {
        if (start_cells[i]) start_state.Set(i, cells[i]);
      }
      init = true;
    }
    
  public:
    Sudoku()
      : cells({{ 0,1,2, 3,4,5, 6,7,8, 
                 5,7,4, 6,0,8, 1,2,3, 
                 3,8,6, 1,7,2, 0,5,4, 
                 8,2,0, 7,3,6, 4,1,5, 
                 1,5,3, 8,2,4, 7,6,0, 
                 6,4,7, 0,5,1, 3,8,2, 
                 7,0,1, 5,8,3, 2,4,6, 
                 4,6,5, 2,1,0, 8,3,7, 
                 2,3,8, 4,6,7, 5,0,1
              }})
      , symbols({{'1','2','3','4','5','6','7','8','9'}})
      , start_state(this), init(false)
    {
      start_cells.fill(false);
    }
    Sudoku(const Sudoku & in)
      : cells(in.cells), start_cells(in.start_cells), symbols(in.symbols)
      , start_state(this), init(false) { ; }
    Sudoku(emp::Random & random, double start_prob=1.0) : start_state(this), init(false) {
      RandomizeCells(random);
      RandomizeStart(random, start_prob);
    }
    Sudoku(std::istream & is) : start_state(this), init (false) { Load(is); }
    Sudoku(const std::string & filename) : start_state(this), init(false) { Load(filename); }
    
    ~Sudoku() { ; }

    int GetCell(int id) const { return cells[id]; }
    bool GetStart(int id) const { return start_cells[id]; }
    char GetCellSymbol(int id) const { return start_cells[id] ? symbols[cells[id]] : '-'; }
    
    const std::array<int,81> & GetCells() const { return cells; }
    const std::array<bool,81> & GetStartCells() const { return start_cells; }
    const std::array<char,9> & GetSymbols() const { return symbols; }
    const SudokuState & GetState() const {
      if (init == false) InitStartState();
      return start_state;
    }

    void SetStart(int id, bool new_start=true) {
      init = false;                 // If this puzzle was initialized, it no longer is.
      start_cells[id] = new_start;
    }
    void MutateStart(emp::Random & random, double toggle_p=0.015) {
      init = false;                 // If this puzzle was initialized, it no longer is.
      for (int i = 0; i < 81; i++) {
        if (random.P(toggle_p)) start_cells[i] = !start_cells[i];
      }
    }

    double CalcSimpleFitness() {
      const auto & profile = CalcProfile();
      return (double) profile.GetSize() + (profile.IsSolved() ? 0 : 100);
    }
    
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

    // Print the current version of this puzzle; by default show start state only.
    void Print(bool full=false, std::ostream & out=std::cout) override;

    // Calculate the full solving profile based on the other techniques.
    const PuzzleProfile & CalcProfile() override;

  };

} // END pze namespace

#endif
