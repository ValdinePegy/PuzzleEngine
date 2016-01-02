#include "EA/Population.h"
#include "tools/Random.h"
#include "web/web.h"

#include "../Sudoku.h"

namespace UI = emp::web;

UI::Document doc("emp_base");
emp::Random rng;
emp::EA::Population<pze::Sudoku> pop;

const int pop_size = 1000;
const double mut_rate = 0.015;
int update = 0;

void DoRunStep() {
  // Mutate existing population
  for (int i = 1; i < pop.GetSize(); i++) {
    pop[i].MutateStart(rng, mut_rate);
  }

  // Do a round of selection.
  pop.EliteSelect( [](pze::Sudoku* s){return s->CalcSimpleFitness();}, 1, 1);
  pop.TournamentSelect( [](pze::Sudoku* s){return s->CalcSimpleFitness();},
                        4, rng, pop_size-1);

  // Collect the top Profile
  auto & profile = pop[0].CalcProfile();
  
  // Print the current status
  auto stats = doc.Table("stats");
  stats.GetCell(0, 1).Clear() << update;
  stats.GetCell(1, 1).Clear();
  for (int i = 0; i < profile.GetSize(); i++) {
    stats << profile.GetLevel(i) << ":" << profile.GetCount(i) << " ";
  }
  stats.GetCell(2, 1).Clear() << pop[0].CalcSimpleFitness();

  // Move to the next generation.
  pop.Update();
  ++update;

  stats.Redraw();
}

extern "C" int main()
{
  UI::Initialize();

  auto & anim = doc.AddAnimation("run", DoRunStep);

  auto play_but = doc.AddButton([&anim]{
      anim.ToggleActive();
      auto but = doc.Button("toggle_run");
      if (anim.GetActive()) but.Label("Pause");
      else but.Label("Start");
    }, "Pause", "toggle_run");
  
  auto stats = doc.AddTable(3, 2, "stats");
  stats.AddHeader(0, 0, "Generation");
  stats.AddHeader(1, 0, "Profile");
  stats.AddHeader(2, 0, "Fitness");
  
  pze::Sudoku puz;
  pop.Insert(puz, pop_size);
  
  anim.Start();
  
  return 0;
}
