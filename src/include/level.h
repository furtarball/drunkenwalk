#ifndef LEVEL_H
#define LEVEL_H
#include "config.h"
#include "entity.h"
#include "map.h"
#include "mapgen.h"
#include <array>
#include <cmath>
#include <concepts>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <typeinfo>
#include <vector>

class EntitiesArray {
	std::vector<std::shared_ptr<Entity>> data;
	size_t doors = 0;
	size_t items = 0;
	bool players = 0;
	size_t mobs = 0;

	public:
	EntitiesArray() = default;
	// iterators; standard vector iterator invalidation rules apply
	// this order ensures player is always on top
	auto begin() { return data.begin(); }
	auto end() { return data.end(); }
	auto door0() { return begin(); }
	auto item0() { return door0() + doors; }
	auto player() { return item0() + items; }
	auto mob0() { return player() + players; }
	auto door_end() { return item0(); }
	auto item_end() { return player(); }
	auto mob_end() { return end(); }

	void insert(std::shared_ptr<Player>);
	void insert(std::shared_ptr<Enemy>);
	void insert(std::shared_ptr<Door>);
	void insert(std::shared_ptr<Item>);
	decltype(data)::iterator erase(decltype(data)::iterator);
};

using Seed = std::array<unsigned, 2>;

class Level {
	public:
	Map map;
	EntitiesArray entities;
	std::seed_seq seed;
	std::shared_ptr<Player> player;
	enum MapType { CAVE_REGULAR, CAVE_CORRIDOR, MAPTYPES };
	std::unique_ptr<MapGenerator> gen;
	Level(size_t map_w, size_t map_h, Seed&, std::shared_ptr<Player>&,
		const Config&);
	void placeDoors();
	void populate();
	void drunkenWalk();
	void generateMap();

	bool collision(const Position);
	bool grab(); // returns true if an item was picked up
	void fight();
};

#endif
