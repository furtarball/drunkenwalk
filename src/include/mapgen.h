#ifndef MAPGEN_H
#define MAPGEN_H
#include <vector>
#include <memory>
#include <random>
#include <array>
#include "level.h"
#include "map.h"
#include "entity.h"

class EntitiesArray;

class MapGenerator {
protected:
  std::mt19937 rng;
  Map& map;
  EntitiesArray& entities;
  Position randomPos();
public:
  MapGenerator(std::seed_seq& s, Map& m, EntitiesArray& e)
    : rng(s), map(m), entities(e) {}
  virtual void generateMap() = 0;
  virtual void populate() = 0;
  virtual ~MapGenerator() = default;
};

class TargetedDrunkenWalk : public MapGenerator {
public:
  TargetedDrunkenWalk(std::seed_seq& s, Map& m, EntitiesArray& e)
    : MapGenerator(s, m, e) {}
  void generateMap();
  void populate();
  Position randomDoorPos();
  void placeDoors();
  std::array<double, 4> distribution(Position&, Position&);
  unsigned choose(std::mt19937&, std::array<double, 4>&);
  void drunkenWalk();
};

class DrunkenWalk : public MapGenerator {
  static const unsigned STEPS = 1024;
public:
  DrunkenWalk(std::seed_seq& s, Map& m, EntitiesArray& e)
    : MapGenerator(s, m, e) {}
  void generateMap();
  void populate();
  void drunkenWalk();
};

#endif
