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

    int GetSize() const { return (int) counts.size(); }
    int GetLevel(int id) const { return levels[id]; }
    int GetCount(int id) const { return counts[id]; }
    void AddMoves(int level, int count) {
      levels.push_back(level);
      counts.push_back(count);
    }

    void Clear() {
      levels.resize(0);
      counts.resize(0);
    }

    void Print(std::ostream & out=std::cout) const {
      for (int i = 0; i < (int) levels.size(); i++) {
        out << levels[i] << ":" << counts[i] << " ";
      }
      out << std::endl;
    }
  };


  class PuzzleState {
  public:
    PuzzleState() { ; }
    virtual ~PuzzleState() { ; }

    virtual void Clear() { ; }
    virtual void Set(int, int) { ; }
    virtual void Block(int, int) { ; }
    virtual void Move(const PuzzleMove &) { ; }
    virtual void Move(const std::vector<PuzzleMove> & moves) {
      for (auto & move : moves) Move(move);
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
    virtual void Print(bool full=false, std::ostream & out=std::cout) = 0;
  };
  
}

#endif
