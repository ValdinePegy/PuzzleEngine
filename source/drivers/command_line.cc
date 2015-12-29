//  This file is part of PuzzleEngine, https://github.com/mercere99/PuzzleEngine/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Main file to run the command-line version of PuzzleEngine

#include <iostream>
#include "../Sudoku.h"
#include "EA/Population.h"

int main()
{
  emp::EA::Population<pze::Sudoku> pop;

  // pze::Sudoku puz("puzzles/blank.puz");
  // pze::Sudoku puz("puzzles/test2.puz");
  // pze::Sudoku puz("puzzles/wikipedia.puz");
  pze::Sudoku puz("puzzles/letters.puz");
  emp::Random random;

  pop.Insert(puz, 100);

  for (int update = 0; update < 1000; update++) {
    for (int i = 1; i < pop.GetSize(); i++) pop[i].MutateStart(random, 0.015);
    pop.EliteSelect( [](pze::Sudoku* s){return s->CalcSimpleFitness();}, 1, 1);
    pop.TournamentSelect( [](pze::Sudoku* s){return s->CalcSimpleFitness();}, 2, random, 99);
    std::cout << update << " : " << pop[0].CalcSimpleFitness() << std::endl;
    pop.Update();
  }
    
  exit(0);
  
  
  // puz.Shuffle(random);
  // puz.Print();

  auto state = puz.GetState();
  // state.ForceSolve();

  state.Print();
  auto moves = state.Solve_FindLastCellState();
  std::cout << "moves = " << moves.size() << std::endl;
  state.OK();

  state.Move(moves);

  state.Print();
  moves = state.Solve_FindLastCellState();
  std::cout << "moves = " << moves.size() << std::endl;
  state.OK();


  auto & profile = puz.CalcProfile();

  for (int i = 0; i < profile.GetSize(); i++) {
    std::cout << profile.GetLevel(i) << " : " << profile.GetCount(i) << std::endl;
  }

  puz.Print();
}
