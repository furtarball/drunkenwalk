#ifndef LEVEL_H
#define LEVEL_H
#include <vector>
#include <random>
#include <memory>
#include <iostream>
#include <array>
#include <fstream>
#include <cmath>
#include "map.h"
#include "entity.h"

class Level {
public:
  Map map;
  std::vector<std::shared_ptr<Entity>> entities;
  size_t nMobs, nDoors, nItems;
  std::seed_seq seed;
  std::mt19937 rng;
  std::shared_ptr<Player>& player;
  static const unsigned STEPS = 10000;
  Level(std::array<unsigned, 2>&, std::shared_ptr<Player>&);
  void populate();
  Position randomizePosition();
  void drunkenWalk();
  void generateMap();
  bool collision(const Position);
  void grab();
  void fight();
};

#endif
