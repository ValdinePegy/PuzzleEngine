#include "Sudoku.h"

namespace pze {

  ////////////////////
  //  SudokuState

  constexpr int SudokuState::members[NUM_REGIONS][9];          // Cells in each region
  constexpr int SudokuState::regions[NUM_CELLS][3];            // Regions for each cell
  constexpr int SudokuState::links[NUM_CELLS][20];             // Other cells across regions
  constexpr int SudokuState::next_opt[512];                    // Which option is next?
  constexpr int SudokuState::opts_count[512];                  // How many options are there?
  constexpr int SudokuState::overlaps[NUM_OVERLAPS][3];        // Cells in each overlap
  constexpr int SudokuState::square_overlaps[18][3];           // Overlaps in each square region
  constexpr int SudokuState::overlap_regions[NUM_OVERLAPS][2]; // Regions for each overlap
                                       
  // A method to clear out all of the solution info when starting a new solve attempt.
  void SudokuState::Clear()
  {
    value.fill(-1);
    options.fill(511);  // Set all options to one.  or 0b111111111
  }

  // Set the value of an individual cell; remove option from linked cells.
  // Return true/false based on whether progress was made toward solving the puzzle.
  void SudokuState::Set(int cell, int state)
  {
    emp_assert(cell >= 0 && cell < NUM_CELLS);    // Make sure cell is in a valid range.
    emp_assert(state >= 0 && state < NUM_STATES); // Make sure state is in a valid range.

    if (value[cell] == state) return;      // If state is already set, SKIP!

    emp_assert(HasOption(cell,state));     // Make sure state is allowed.
    value[cell] = state;                   // Store found value!
    options[cell] = 0;                     // No options available to locked cells.
    
    // Now make sure this state is blocked from all linked cells.
    for (int id : links[cell]) Block(id, state);
  }

  // Operate on a "move" object.
  void SudokuState::Move(const PuzzleMove & move)
  {
    emp_assert(move.GetID() >= 0 && move.GetID() < NUM_CELLS, move.GetID());
    emp_assert(move.GetState() >= 0 && move.GetState() < NUM_STATES, move.GetState());
    
    switch (move.GetType()) {
    case PuzzleMove::SET_STATE:   Set(move.GetID(), move.GetState());   break;
    case PuzzleMove::BLOCK_STATE: Block(move.GetID(), move.GetState()); break;
    default:
      emp_assert(false);   // One of the previous move options should have been triggered!
    }
  }
  

  // Print the current state of the puzzle, including all options available.
  void SudokuState::Print(const std::array<char,NUM_STATES> & symbols, std::ostream & out)
  {
    out << " +-----------------------+-----------------------+-----------------------+"
        << std::endl;;
    for (int r = 0; r < 9; r++) {       // Puzzle row
      for (int s = 0; s < 9; s+=3) {    // Subset row
        for (int c = 0; c < 9; c++) {   // Puzzle col
          int id = r*9+c;
          if (c%3==0) out << " |";
          else out << "  ";
          if (value[id] == -1) {
            out << " " << (char) (HasOption(id,s)   ? symbols[s] : '.')
                << " " << (char) (HasOption(id,s+1) ? symbols[s+1] : '.')
                << " " << (char) (HasOption(id,s+2) ? symbols[s+2] : '.');
          } else {
            if (s==0) out << "      ";
            if (s==3) out << "   " << symbols[value[id]] << "  ";
            if (s==6) out << "      ";
            // if (s==0) out << " /   \\";
            // if (s==3) out << " | " << symbols[value[id]] << " |";
            // if (s==6) out << " \\   /";
          }
        }
        out << " |" << std::endl;
      }
      if (r%3==2) {
        out << " +-----------------------+-----------------------+-----------------------+";
      }
      else {
        out << " |                       |                       |                       |";
      }
      out << std::endl;
    }
  }
    
  void SudokuState::Print(std::ostream & out)
  {
    // If no character map is provided, use default for Sudoku
    Print(puzzle->GetSymbols(), out);
  }
  

  bool SudokuState::ForceSolve(int start)
  {
    emp_assert(start >= 0 && start <= NUM_CELLS);
    
    // Advance the start position until we find a cell with a choice to be made.
    while (start < NUM_CELLS) {
      const int opt_count = CountOptions(start);
      if (opt_count == 0 && !IsSet(start)) return false;      // No option & unlocked -> backtrack!
      else if (opt_count == 1) Set( start, FindNext(start) ); // One option -> lock it!
      else if (opt_count > 1) break;                          // Multiple options -> move on!
    
      start++;   // Must have locked option, increment and keep looping!
    }

    // If we've made it through all positions stop here.
    if (start == 81) return true;
    
    // Step through possibilities of first cell with multiple options.
    for (int i = 0; i < NUM_STATES; i++) {
      if (HasOption(start,i) == false) continue;  // Skip values that are not an option.
      
      SudokuState backup_state(*this);    // backup the current state.
      Set(start, i);                      // set this cell to next possible value.
      bool solved = ForceSolve(start+1);  // continue attempt to solve!
      if (solved) return true;            // if solved, we're done!
      *this = backup_state;               // otherwise, restore from backup and loop.
    }

    // If we made it this far, we were unable to find a solution.
    return false;
  }

  // If there's only one state a cell can be, pick it!
  std::vector<PuzzleMove> SudokuState::Solve_FindLastCellState()
  {
    std::vector<PuzzleMove> moves;

    // For each cell, check if it has only one state left.
    for (int i = 0; i < NUM_CELLS; i++) {
      if (CountOptions(i) == 1) {
        // Find last value.
        moves.emplace_back(PuzzleMove::SET_STATE, i, FindNext(i));
      }
    }

    return moves;
  }

  // If there's only one cell that can have a certain state in a region, choose it!
  std::vector<PuzzleMove> SudokuState::Solve_FindLastRegionState()
  {
    std::vector<PuzzleMove> moves;

    // For each region, check if it has any states with only one available cell.
    for (const auto & region : members) {
      uint32_t opt_any = 0;     // Is a state an option in ANY cell?
      uint32_t opt_multi = 0;   // Is a state an option in MULTIPLE cells?
      for (const int c : region) {
        opt_multi |= (options[c] & opt_any);  // If we already had an option AND see a new one.
        opt_any |= options[c];                // Mark these options as possible.
      }
      const uint32_t opt_once = opt_any & ~opt_multi;

      // If any options are only available in one cell, find them and lock them in.
      if (opt_once) {
        for (const int c : region) {
          const uint32_t opt_unique = options[c] & opt_once;
          if (opt_unique) {
            moves.emplace_back(PuzzleMove::SET_STATE, c, next_opt[opt_unique]);
          }
        }
      }
    }
    
    return moves;
  }

  // If only cells that can have a state in region A are all also in region
  // B, no other cell in region B can have that state as a possibility.
  std::vector<PuzzleMove> SudokuState::Solve_FindRegionOverlap()
  {
    std::vector<PuzzleMove> moves;

    // std::array<uint32_t, NUM_OVERLAPS> overlap_options;
    // for (int i = 0; i < 54; i++) {
    // }

    return moves;
  }
    
  // If K cells are all limited to the same K states, eliminate those states
  // from all other cells in the same region.
  std::vector<PuzzleMove> SudokuState::Solve_FindLimitedCells()
  {
    std::vector<PuzzleMove> moves;
    return moves;
  }
    
  // Eliminate all other possibilities from K cells if they are the only
  // ones that can possess K states in a single region.
  std::vector<PuzzleMove> SudokuState::Solve_FindLimitedStates()
  {
    std::vector<PuzzleMove> moves;
    return moves;
  }

  // If there are X rows (cols) where a certain state can only be in one of 
  // X cols (rows), then no other row in this cols can be that state.
  std::vector<PuzzleMove> SudokuState::Solve_FindSwordfish()
  {
    std::vector<PuzzleMove> moves;
    return moves;
  }

  
  bool SudokuState::OK()
  {    
    // Make sure we are associated with a puzzle.
    emp_assert(puzzle != nullptr);
    
    // Run tests on each cell...
    for (int cell = 0; cell < 81; cell++) {
      // Make sure any set values are the only allowed option.
      if (value[cell] != -1) {
        emp_assert(options[cell] == 0);
      }
      
      // Make sure that the opt_counts are equal to the number of options available.
      int count = 0;
      for (int i = 0; i < 9; i++) {
        if (HasOption(cell,i)) count++;
      }
      emp_assert(CountOptions(cell) == count);

      // Make sure this state is consistant with its puzzle.
      const int pstate = puzzle->GetCell(cell);
      if (pstate >= 0) {
        emp_assert(pstate == value[cell] || HasOption(cell,pstate) == true);
      }
    }

    return true;
  }
  
  
  ////////////////////
  //  Sudoku

  bool Sudoku::Load(std::istream & is)
  {
    init = false;                 // If this puzzle was initialized, it no longer is.

    cells.fill(-1);               // Initialize all cells as unset.
    symbols.fill(0);              // Reset all symbols used.
    std::array<int, 128> sym_id;  // Which id is associated with each symbol?
    sym_id.fill(-2);
    sym_id['-'] = -1;             // A dash should be used as an empty cell.
    int sym_count = 0;            // How many unique symbols have we seen?
    
    // Step through loading each character from the input stream.
    int load_count = 0;
    char cur_char;
    while (load_count < 81) {
      is >> cur_char;
      
      // If this is whitespace, keep going.
      if (emp::is_whitespace(cur_char)) continue;

      int cur_id = sym_id[cur_char];
      if (cur_id <= -2) {                 // If this symbol wasn't found...
        if (sym_count >= 9) return false; //  Make sure we have room for another ID
        cur_id = sym_count++;             //  Determine the new ID for this character.
        symbols[cur_id] = cur_char;       //  Store this character
        sym_id[cur_char] = cur_id;        //  Store the new ID
      }
      
      // Otherwise load this character into the tables.
      cells[load_count] = cur_id;              // Store the current ID.
      start_cells[load_count] = (cur_id >= 0); // Any non-empty cell should be a start state.
      
      load_count++;
    }

    // If we need more symbols, fill them in.
    cur_char = '1';
    while (sym_count < 9) {
      if (sym_id[cur_char] == -2) {
        symbols[sym_count++] = cur_char;
      }
      cur_char++;
    }

    // If any of the cells are still empty, fill them in by brute force
    // (but don't mark them as starting cells!)
    auto state = GetState();
    state.ForceSolve();
    for (int i = 0; i < 81; i++) {
      if (cells[i] == -1) {
        cells[i] = state.GetValue(i);
      }
    }
    
    return true;
  }

  void Sudoku::RandomizeCells(emp::Random & random)
  {
    init = false;                 // If this puzzle was initialized, it no longer is.
    // @CAO Do This!!!
    // cells.fill(-1);                        // Clear out current cells
    // solve.Clear();                         // Clear out helper info
    // for (int i=0; i<9; i++) {              // Setup first cells to be 0-8
    //   cells[i] = i;                        //   set cur cell id
    //   solve.options[i].fill(false);        //   cross out most options
    //   solve.options[i][i] = true;          //   ...except the one chosen.
    //   for (int r : regions[i]) {           //   loop through all regions for this cell
    //     for (int c : members[r]) {         //     loop through other cells in each region
    //       if (i==c) continue;              //       ignore current cell
    //       if (solve.options[c][i]) {       //       if new value was previously okay...
    //         solve.opt_count[c]--;          //         note that one fewer option is available
    //         solve.options[c][i] = false;   //         and mark off the option
    //       }
    //     }
    //   }
    // }
    // RandomizeCells_step(random, 9);
  }

  // Shuffle will:
  // * Remap all symbols
  // * Shuffle rows/columns within sets of three
  // * Shuffle rows/columns OF sets of three
  void Sudoku::Shuffle(emp::Random & random)
  {
    init = false;                 // If this puzzle was initialized, it no longer is.
    
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
  
  void Sudoku::RandomizeStart(emp::Random & random, double start_prob)
  {
    emp_assert(start_prob >= 0.0 && start_prob <= 1.0);

    init = false;                 // If this puzzle was initialized, it no longer is.
    for (int i = 0; i < 81; i++) start_cells[i] = random.P(start_prob);
  }

  void Sudoku::Print(bool full, std::ostream & out)
  {
    for (int id = 0; id < 81; id++) {
      if (id % 3 == 0) out << ' ';
      if (full || start_cells[id]) {
        out << ' ' << symbols[ cells[id] ];
      } else {
        out << " -";
      }
      if (id % 9 == 8) out << '\n';
      if (id == 26 || id == 53) out << '\n';
    }
  }

  
  // Calculate the full solving profile based on the other techniques.
  const PuzzleProfile & Sudoku::CalcProfile()
  {
    profile.Clear();  // Reset the profile if already calculated.

    // Setup a starting state for solving the puzzle.
    SudokuState state = GetState();

    while (true) {    
      auto moves = state.Solve_FindLastCellState();
      if (moves.size() > 0) {
        state.Move(moves);
        profile.AddMoves(0, moves.size());
        continue;
      }
      
      moves = state.Solve_FindLastRegionState();
      if (moves.size() > 0) {
        state.Move(moves);
        profile.AddMoves(1, moves.size());
        continue;
      }
      
      break;  // No new moves found!
    }

    profile.SetSolved(state.IsSolved());
    // state.OK();
    
    return profile;
  }


  
}
