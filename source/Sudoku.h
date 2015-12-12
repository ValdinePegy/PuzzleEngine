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

#include <vector>
#include "tools/BitSet.h"
#include "Puzzle.h"

namespace pze {
  class Sudoku : public Puzzle {
  private:
    const int regions[2][9] = {{0,1,2,3,4,5,6,7,8},{9,10,11,12,13,14,15,16,17}};

    int cells[81];              // What is the final solution?
    emp::BitSet<81> start_cell; // Is each cell visible in the starting layout?

  public:
    Sudoku() { ; }
    ~Sudoku() { ; }

  };
}

#endif
