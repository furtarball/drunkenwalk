#include "include/mapgen.h"
#include "include/entity.h"
#include "include/level.h"
#include "include/map.h"
#include <array>
#include <fstream>
#include <functional>
#include <memory>
#include <vector>

Position MapGenerator::randomPos() {
	Position p;
	bool withinBorders, occupied;
	do { // picks random position until a valid and unoccupied one is found
		p = Position(
			(rng() % (map.width - 3)) + 1, (rng() % (map.height - 3)) + 1);
		withinBorders = (map[p.getX()][p.getY()] == 1);
		occupied = false;
		if (withinBorders)
			for (auto&& i : entities) {
				if (p == i->position) {
					occupied = true;
					break;
				}
			}
	} while (!withinBorders || occupied);
	return p;
}
void TargetedDrunkenWalk::generateMap() {
	placeDoors();
	map.zero();
	drunkenWalk();
	map.smoothen(0, 1);
	map.drawWall();
	map.zeroBorder();
}
void TargetedDrunkenWalk::populate(
	const nlohmann::json& itemTypes, const nlohmann::json& enemyTypes) {
	(*(entities.player()))->position = randomPos();
	size_t mobs = map.countWalkable() / ((rng() % 100) + 50);
	for (size_t i = 0; i < mobs; i++)
		entities.insert(std::make_shared<Enemy>(
			randomPos(), enemyTypes[rng() % enemyTypes.size()]));
	size_t items = mobs;
	for (size_t i = 0; i < items; i++)
		entities.insert(std::make_shared<Item>(
			randomPos(), itemTypes[rng() % itemTypes.size()]));
}
Position TargetedDrunkenWalk::randomDoorPos() {
	Position p;
	bool occupied;
	std::array<std::function<Position()>, 4> pick = {
		[&]() { return Position(1, (rng() % (map.height - 2)) + 1); },
		[&]() { return Position((rng() % (map.width - 2)) + 1, 1); },
		[&]() {
			return Position(map.width - 2, (rng() % (map.height - 2)) + 1);
		},
		[&]() {
			return Position((rng() % (map.width - 2)) + 1, map.height - 2);
		}};
	do {
		p = pick[rng() % 4]();
		occupied = false;
		for (auto&& i : entities) {
			if (p == i->position) {
				occupied = true;
				break;
			}
		}
	} while (occupied);
	return p;
}

void TargetedDrunkenWalk::placeDoors() {
	size_t doors = (rng() % 4) + 1;
	Door::count = 0;
	for (size_t i = 0; i < doors; i++)
		entities.insert(std::make_shared<Door>(randomDoorPos()));
}

std::array<double, 4> TargetedDrunkenWalk::distribution(
	Position& s, Position& f) {
	const double P = 0.6; // probability of choosing the correct direction
	std::array<double, 4> p = {0};
	if (f.getY() < s.getY())
		p[0] = P / 2;
	if (f.getY() > s.getY())
		p[1] = P / 2;
	if (f.getX() < s.getX())
		p[2] = P / 2;
	if (f.getX() > s.getX())
		p[3] = P / 2;
	double sum = p[0] + p[1] + p[2] + p[3];
	if (sum == (P / 2)) {
		for (auto&& i : p)
			if (i == 0)
				i = (1.0 - sum - (P / 2)) / 3;
			else
				i = P;
	} else if (sum == P) {
		for (auto&& i : p)
			if (i == 0)
				i = (1.0 - sum) / 2;
	}
	return p;
}

unsigned TargetedDrunkenWalk::choose(
	std::mt19937& rng, std::array<double, 4>& p) {
	std::array<double, 4> csum = {
		p[0], p[0] + p[1], p[0] + p[1] + p[2], p[0] + p[1] + p[2] + p[3]};
	double r = static_cast<double>(rng()) / rng.max();
	for (size_t i = 0; i < 4; i++) {
		if (r < csum[i])
			return i;
	}
	return 5; // TODO replace this with real error handling
}

void TargetedDrunkenWalk::drunkenWalk() {
	std::array movements{
		&Position::up, &Position::down, &Position::left, &Position::right};
	for (auto i = entities.door0(); i < entities.door_end(); i++) {
		Position p =
			map.getStart(); // TODO why does Map have a start pos attribute?
		map[p.getX()][p.getY()] = 1;
		Position finish = (*i)->position;
		while (p != finish) {
			std::array<double, 4> probability = distribution(p, finish);
			unsigned dir = choose(rng, probability);
			p = (p.*(movements[dir]))(map);
			map[p.getX()][p.getY()] = 1;
		}
	}
}

void DrunkenWalk::generateMap() {
	map.zero();
	drunkenWalk();
	map.smoothen(1, 8);
	map.smoothen(0, 8);
	map.smoothen(0, 3);
	map.drawWall();
	map.zeroBorder();
}

void DrunkenWalk::populate(
	const nlohmann::json& itemTypes, const nlohmann::json& enemyTypes) {
	size_t doors = (rng() % 4) + 1;
	Door::count = 0;
	for (size_t i = 0; i < doors; i++)
		entities.insert(std::make_shared<Door>(randomPos()));
	(*(entities.player()))->position = randomPos();
	size_t mobs = map.countWalkable() / ((rng() % 100) + 50);
	for (size_t i = 0; i < mobs; i++)
		entities.insert(std::make_shared<Enemy>(
			randomPos(), enemyTypes[rng() % enemyTypes.size()]));
	size_t items = mobs;
	for (size_t i = 0; i < items; i++)
		entities.insert(std::make_shared<Item>(
			randomPos(), itemTypes[rng() % itemTypes.size()]));
}

void DrunkenWalk::drunkenWalk() {
	// array of pointers to member functions of class Position
	std::array movements{
		&Position::up, &Position::down, &Position::left, &Position::right};
	Position p = map.getStart();
	for (size_t i = 0; i < STEPS; i++) {
		map[p.getX()][p.getY()] = 1;
		unsigned dir = rng() & 3;
		p = (p.*(movements[dir]))(map);
	}
}
