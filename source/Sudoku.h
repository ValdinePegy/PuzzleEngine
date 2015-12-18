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
#include <vector>
#include "tools/assert.h"
#include "tools/Random.h"
#include "Puzzle.h"

namespace pze {
  class SudokuMove : public PuzzleMove {
  private:
    int cell_id;
    int state;

  public:
    SudokuMove() { ; }
    ~SudokuMove() { ; }

    int GetID() const override { return cell_id; }
    int GetState() const override { return state; }
  };



  struct SudokuState : public PuzzleState {
    std::array<char,81> value;                   // Known value for cells; -1 = unknown.
    std::array<char,81> opt_count;               // How many options does each cell have?
    std::array<std::array<bool,9>, 81> options;  // Which options are available to each cell?

    // A method to clear out all of the solution info when starting a new solve attempt.
    void Clear() override {
      value.fill(-1);
      opt_count.fill(9);
      options.fill({1,1,1, 1,1,1, 1,1,1});
    }

    bool Set(int cell, int state) override {
      emp_assert(options[cell][state] == true);  // Make sure state is allowed.
      bool progress = value[cell] == -1;
      value[cell] = state;                       // Store found value!
      opt_count[cell] = 1;                       // This is the only option now.
      options[cell] = {0,0,0,0,0,0,0,0,0};
      options[cell][state] = 1;
      return progress;
    }

    bool Block(int cell, int state) override {
      if (options[cell][state]) { // If this value was previously an option, remove it.
        options[cell][state] = false;
        opt_count[cell]--;
        return true;
      }
      return false;
    }
    
    bool Move(const PuzzleMove & move) override {
      switch (move.GetType()) {
      case PuzzleMove::SET_STATE:   return Set(move.GetID(), move.GetState());
      case PuzzleMove::BLOCK_STATE: return Block(move.GetID(), move.GetState());
      }
      
      emp_assert(false);   // One of the previous move options should have been triggered!
      return false;
    }
    

  };

  
  
  class Sudoku : public Puzzle {
  private:
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

    // Core puzzle info
    std::array<int,81> cells;         // What is the full solution?
    std::array<bool,81> start_cells;  // Is each cell visible at the start?

    // Solve info
    SudokuState solve;                // Current solve state.

    
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

      start_cells.fill(true);
    }
    Sudoku(const Sudoku &) = default;
    Sudoku(emp::Random & random, double start_prob=1.0) {
      RandomizeCells(random);
      RandomizeStart(random, start_prob);
    }
    ~Sudoku() { ; }

    void RandomizeCells(emp::Random & random) {
      cells.fill(-1);                        // Clear out current cells
      solve.Clear();                     // Clear out helper info
      for (int i=0; i<9; i++) {              // Setup first cells to be 0-8
        cells[i] = i;                        //   set cur cell id
        solve.options[i].fill(false);           //   cross out most options
        solve.options[i][i] = true;             //   ...except the one chosen.
        for (int r : regions[i]) {           //   loop through all regions for this cell
          for (int c : members[r]) {         //     loop through other cells in each region
            if (i==c) continue;              //       ignore current cell
            if (solve.options[c][i]) {          //       if new value was previously okay...
              solve.opt_count[c]--;          //         note that one fewer option is available
              solve.options[c][i] = false;      //         and mark off the option
            }
          }
        }
      }
      RandomizeCells_step(random, 9);
    }

    // Shuffle will:
    // * Remap all symbols
    // * Shuffle rows/columns within sets of three
    // * Shuffle rows/columns OF sets of three
    void Shuffle(emp::Random & random) {
      // Remap all states.
      std::vector<int> remap( random.GetPermutation(9) );
      for (int & c : cells) c = remap[c];

      // Shuffle rows
      std::vector<int> row_blockset_map( random.GetPermutation(3) );
      std::vector<int> row_block0_map( random.GetPermutation(3) );
      std::vector<int> row_block1_map( random.GetPermutation(3) );
      std::vector<int> row_block2_map( random.GetPermutation(3) );
      std::array<int, 9> row_map;
      row_map[0] = row_blockset_map[0]*3 + row_block0_map[0];
      row_map[1] = row_blockset_map[0]*3 + row_block0_map[1];
      row_map[2] = row_blockset_map[0]*3 + row_block0_map[2];
      row_map[3] = row_blockset_map[1]*3 + row_block1_map[0];
      row_map[4] = row_blockset_map[1]*3 + row_block1_map[1];
      row_map[5] = row_blockset_map[1]*3 + row_block1_map[2];
      row_map[6] = row_blockset_map[2]*3 + row_block2_map[0];
      row_map[7] = row_blockset_map[2]*3 + row_block2_map[1];
      row_map[8] = row_blockset_map[2]*3 + row_block2_map[2];

      std::array<int, 81> tmp_cells;
      std::array<bool, 81> tmp_start;
      for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
          tmp_cells[r*9 + c] = cells[row_map[r]*9 + c];
          tmp_start[r*9 + c] = start_cells[row_map[r]*9 + c];
        }
      }

      // Shuffle cols
      std::vector<int> col_blockset_map( random.GetPermutation(3) );
      std::vector<int> col_block0_map( random.GetPermutation(3) );
      std::vector<int> col_block1_map( random.GetPermutation(3) );
      std::vector<int> col_block2_map( random.GetPermutation(3) );
      std::array<int, 9> col_map;
      col_map[0] = col_blockset_map[0]*3 + col_block0_map[0];
      col_map[1] = col_blockset_map[0]*3 + col_block0_map[1];
      col_map[2] = col_blockset_map[0]*3 + col_block0_map[2];
      col_map[3] = col_blockset_map[1]*3 + col_block1_map[0];
      col_map[4] = col_blockset_map[1]*3 + col_block1_map[1];
      col_map[5] = col_blockset_map[1]*3 + col_block1_map[2];
      col_map[6] = col_blockset_map[2]*3 + col_block2_map[0];
      col_map[7] = col_blockset_map[2]*3 + col_block2_map[1];
      col_map[8] = col_blockset_map[2]*3 + col_block2_map[2];

      for (int r = 0; r < 9; r++) {
        const int R = r*9;
        for (int c = 0; c < 9; c++) {
          cells[R + c] = tmp_cells[R + col_map[c]];
          start_cells[R + c] = tmp_start[R + col_map[c]];
        }
      }
    }

    void RandomizeStart(emp::Random & random, double start_prob=1.0) {
      for (int i = 0; i < 81; i++) start_cells[i] = random.P(start_prob);
    }

    void Print(std::ostream & out=std::cout) override {
      for (int id = 0; id < 81; id++) {
        if (id % 3 == 0) out << ' ';
        out << ' ' << cells[id];
        if (id % 9 == 8) out << '\n';
        if (id == 26 || id == 53) out << '\n';
      }
    }
  };
}

#endif
