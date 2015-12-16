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

  class PuzzleMove {
  public:
    enum MoveType {
      SET_STATE,
      BLOCK_STATE
    };

  private:
    MoveType type;

  public:
    PuzzleMove() { ; }
    virtual ~PuzzleMove() { ; }

    MoveType GetType() const { return type; }
    virtual int GetState() const = 0;
    virtual int GetID() const = 0;
  };


  
  class Puzzle {
  public:
    Puzzle() { ; }
    virtual ~Puzzle() { ; }

    virtual bool Move(const PuzzleMove & move) = 0;
    bool Move(const std::vector<PuzzleMove> & moves) {
      bool progress = false;
      for (auto & move : moves) progress |= Move(move);
      return progress;
    }
    
    virtual void Print(std::ostream & out=std::cout) = 0;
  };
  
}

#endif
