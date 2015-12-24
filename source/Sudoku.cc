#include "Sudoku.h"

namespace pze {

  ////////////////////
  //  SudokuState

  constexpr int SudokuState::members[27][9];
  constexpr int SudokuState::regions[81][3];
  constexpr int SudokuState::links[81][20];
                                       
  // A method to clear out all of the solution info when starting a new solve attempt.
  void SudokuState::Clear()
  {
    value.fill(-1);
    opt_count.fill(9);
    options.fill({1,1,1, 1,1,1, 1,1,1});
  }
  
  // Set an individual cell in the current state; remove option from linked cells.
  bool SudokuState::Set(int cell, int state)
  {
    emp_assert(options[cell][state] == true);  // Make sure state is allowed.
    bool progress = value[cell] == -1;
    value[cell] = state;                       // Store found value!
    opt_count[cell] = 1;                       // This is the only option now.
    options[cell] = {0,0,0,0,0,0,0,0,0};
    options[cell][state] = 1;
    
    // Now make sure this state is blocked from all linked cells.
    for (int id : links[cell]) Block(id, state);
      
    return progress;
  }

  // Remove a symbol option from a particular cell.
  bool SudokuState::Block(int cell, int state)
  {
    if (options[cell][state]) { // If this value was previously an option, remove it.
      options[cell][state] = false;
      opt_count[cell]--;
      return true;
    }
    return false;
  }

  // Operate on a "move" object.
  bool SudokuState::Move(const PuzzleMove & move)
  {
    switch (move.GetType()) {
    case PuzzleMove::SET_STATE:   return Set(move.GetID(), move.GetState());
    case PuzzleMove::BLOCK_STATE: return Block(move.GetID(), move.GetState());
    }
    
    emp_assert(false);   // One of the previous move options should have been triggered!
    return false;
  }
  

  // Print the current state of the puzzle, including all options available.
  void SudokuState::Print(const std::array<char,9> & symbols, std::ostream & out)
  {
    out << " +-----------------------+-----------------------+-----------------------+"
        << std::endl;;
    for (int r = 0; r < 9; r++) {       // Puzzle row
      for (int s = 0; s < 9; s+=3) {    // Subset row
        for (int c = 0; c < 9; c++) {   // Puzzle col
          int id = r*9+c;
          if (c%3==0) out << " |";
          else out << "  ";
          out << " " << (char) (options[id][s]  ? symbols[s] : '.')
              << " " << (char) (options[id][s+1] ? symbols[s+1] : '.')
              << " " << (char) (options[id][s+2] ? symbols[s+2] : '.');
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
    std::array<char,9> symbols = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    Print(symbols, out);
  }
  

  ////////////////////
  //  Sudoku

  SudokuState Sudoku::GetState()
  {
    SudokuState state(this);
    for (int i = 0; i < 81; i++) {
      if (start_cells[i]) state.Set(i, cells[i]);
    }
    return state;
  }
    
  bool Sudoku::Load(std::istream & is)
  {
    symbols.fill(0);              // Reset all symbols used.
    std::array<int, 128> sym_id;  // Which id is associated with each symbol?
    sym_id.fill(-2);
    sym_id['-'] = -1;             // A dash should be used as an empty cell.
    int sym_count = 0;            // How many unique symbols have we seen?
    
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
    
    return true;
  }

  void Sudoku::RandomizeCells(emp::Random & random)
  {
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
    for (int i = 0; i < 81; i++) start_cells[i] = random.P(start_prob);
  }

  void Sudoku::Print(std::ostream & out)
  {
    for (int id = 0; id < 81; id++) {
      if (id % 3 == 0) out << ' ';
      out << ' ' << cells[id];
      if (id % 9 == 8) out << '\n';
      if (id == 26 || id == 53) out << '\n';
    }
  }
  
  
}
