//  This file is part of PuzzleEngine, https://github.com/mercere99/PuzzleEngine/
//  Copyright (C) Michigan State University, 2015.
//  Released under the MIT Software license; see doc/LICENSE
//
//
//  This class defines a base class for all puzzle instances, regardless of the
//  type of puzzle being used.
//

#ifndef PZE_PUZZLE_H
#define PZE_PUZZLE_H

namespace pze {

  class Puzzle {
  public:
    Puzzle() { ; }
    virtual ~Puzzle() { ; }
  };
  
}

#endif
