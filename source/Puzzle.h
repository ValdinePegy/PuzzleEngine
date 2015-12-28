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

  protected:
    MoveType type;
    int pos_id;
    int state;

    
  public:
    PuzzleMove(MoveType t, int p, int s) : type(t), pos_id(p), state(s) { ; }
    virtual ~PuzzleMove() { ; }

    PuzzleMove & operator=(const PuzzleMove &) = default;

    MoveType GetType() const { return type; }
    int GetID() const { return pos_id; }
    int GetState() const { return state; }
  };


  class PuzzleProfile {
  protected:
    std::vector<int> levels;
    std::vector<int> counts;

  public:
    PuzzleProfile() { ; }
    ~PuzzleProfile() { ; }
    PuzzleProfile & operator=(const PuzzleProfile &) = default;

    int GetSize() { return (int) counts.size(); }
    int GetLevel(int id) { return levels[id]; }
    int GetCount(int id) { return counts[id]; }
    void AddMoves(int level, int count) {
      levels.push_back(level);
      counts.push_back(count);
    }

    void Clear() {
      levels.resize(0);
      counts.resize(0);
    }
  };


  class PuzzleState {
  public:
    PuzzleState() { ; }
    virtual ~PuzzleState() { ; }

    virtual void Clear() { ; }
    virtual bool Set(int, int) { return false; }
    virtual bool Block(int, int) { return false; }
    virtual bool Move(const PuzzleMove &) { return false; }
    virtual bool Move(const std::vector<PuzzleMove> & moves) {
      bool progress = false;
      for (auto & move : moves) progress |= Move(move);
      return progress;
    }

    virtual void Print(std::ostream & out=std::cout) = 0;
  };
  
  
  class Puzzle {
  protected:
    PuzzleProfile profile;            // The solving profile associated with this puzzle.
    
  public:
    Puzzle() { ; }
    virtual ~Puzzle() { ; }
    
    const PuzzleProfile & GetProfile() const { return profile; }

    virtual const PuzzleProfile & CalcProfile() = 0;
    virtual void Print(std::ostream & out=std::cout) = 0;
  };
  
}

#endif
