#ifndef LEVEL_H
#define LEVEL_H
#include <vector>
#include <random>
#include <memory>
#include <iostream>
#include <array>
#include <fstream>
#include <cmath>
#include <typeinfo>
#include "map.h"
#include "entity.h"
#include "mapgen.h"

class EntitiesArray : private std::vector<std::shared_ptr<Entity>> {
  bool players;
  size_t mobs, doors, items;
  public:
  EntitiesArray() : players(0), mobs(0), doors(0), items(0) {}
  // iterators; standard vector iterator invalidation rules apply
  auto begin() { return std::vector<std::shared_ptr<Entity>>::begin(); }
  auto end() { return std::vector<std::shared_ptr<Entity>>::end(); }
  auto player() { return begin(); }
  auto mob0() { return player() + players; }
  auto door0() { return mob0() + mobs; }
  auto item0() { return door0() + doors; }
  auto mob_end() { return door0(); }
  auto door_end() { return item0(); }
  auto item_end() { return end(); }

  void push_back(std::shared_ptr<Player>);
  // these accept traditional pointers to turn into shared_ptrs
  void push_back(Enemy*);
  void push_back(Door*);
  void push_back(Item*);
  iterator erase(iterator);
};

class Level {
public:
  Map map;
  EntitiesArray entities;
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
  bool grab(); // returns true if an item was picked up
  void fight();
};

#endif
