//  This file is part of PuzzleEngine, https://github.com/mercere99/PuzzleEngine/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//  Main file to run the command-line version of PuzzleEngine

#include <iostream>
#include <fstream>
#include "../Sudoku.h"
#include "EA/Population.h"

void DoRun(const pze::Sudoku & puz, emp::Random & random,
           int pop_size, int num_updates, double mut_rate, std::ostream & out_log)
{
  out_log << pop_size 
          << ", " << num_updates
          << ", " << mut_rate;
  
  emp::EA::Population<pze::Sudoku> pop;
  pop.Insert(puz, pop_size);

  for (int update = 0; update < num_updates; update++) {
    for (int i = 1; i < pop.GetSize(); i++) {
      pop[i].MutateStart(random, mut_rate);
    }

    pop.EliteSelect( [](pze::Sudoku* s){return s->CalcSimpleFitness();}, 1, 1);
    pop.TournamentSelect( [](pze::Sudoku* s){return s->CalcSimpleFitness();},
                          2, random, pop_size-1);
    std::cout << update << " : " << pop[0].CalcSimpleFitness() << std::endl;
    pop.Update();
  }

  out_log << ", " << pop[0].CalcSimpleFitness()
          << std::endl;
  pop[0].Print();
  
}

int main()
{
  // pze::Sudoku puz("puzzles/blank.puz");
  // pze::Sudoku puz("puzzles/test2.puz");
  // pze::Sudoku puz("puzzles/wikipedia.puz");
  pze::Sudoku puz("puzzles/letters.puz");
  emp::Random random;

  std::ofstream out("out.log");

  DoRun(puz, random, 100, 1000, 0.015, out);
  exit(0);

  int reps = 10;
  double mut_rates[] = { 0.002, 0.004, 0.0075, 0.015, 0.03, 0.06 };
  
  for (double m : mut_rates) {
    for (int r=0; r < reps; r++) {
      DoRun(puz, random, 100, 1000, m, out);
    }
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
