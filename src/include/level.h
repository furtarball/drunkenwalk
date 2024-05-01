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
#include "mapgen.h"

class Level {
public:
  Map map;
  std::vector<std::shared_ptr<Entity>> entities;
  size_t nMobs, nDoors, nItems;
  std::seed_seq seed;
  std::shared_ptr<Player>& player;
  enum MapType { CAVE_REGULAR, CAVE_CORRIDOR, MAPTYPES };
  std::unique_ptr<MapGenerator> gen;
  Level(std::array<unsigned, 2>&, std::shared_ptr<Player>&);
  void placeDoors();
  void populate();
  void drunkenWalk();
  void generateMap();
  
  bool collision(const Position);
  void grab();
  void fight();
};

#endif
