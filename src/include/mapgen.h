#ifndef MAPGEN_H
#define MAPGEN_H
#include <vector>
#include <memory>
#include <random>
#include <array>
#include "map.h"
#include "entity.h"

class MapGenerator {
protected:
  std::mt19937 rng;
  Map& map;
  std::vector<std::shared_ptr<Entity>>& entities;
  size_t& doors;
  size_t& mobs;
  size_t& items;
  Position randomPos();
public:
  MapGenerator(std::seed_seq& s, Map& m, std::vector<std::shared_ptr<Entity>>& e, size_t& d, size_t& en, size_t& i)
    : rng(s), map(m), entities(e), doors(d), mobs(en), items(i) {}
  virtual void generateMap() = 0;
  virtual void populate() = 0;
  virtual ~MapGenerator() {}
};

class TargetedDrunkenWalk : public MapGenerator {
public:
  TargetedDrunkenWalk(std::seed_seq& s, Map& m, std::vector<std::shared_ptr<Entity>>& e, size_t& d, size_t& en, size_t& i) : MapGenerator(s, m, e, d, en, i) {}
  void generateMap();
  void populate();
  Position randomDoorPos();
  void placeDoors();
  std::array<double, 4> distribution(Position&, Position&);
  unsigned choose(std::mt19937&, std::array<double, 4>&);
  void drunkenWalk();
};

class DrunkenWalk : public MapGenerator {
  static const unsigned STEPS = 10000;
public:
  DrunkenWalk(std::seed_seq& s, Map& m, std::vector<std::shared_ptr<Entity>>& e, size_t& d, size_t& en, size_t& i) : MapGenerator(s, m, e, d, en, i) {}
  void generateMap();
  void populate();
  void drunkenWalk();
};

#endif
