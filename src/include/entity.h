#ifndef ENTITY_H
#define ENTITY_H

#include "map.h"
#include <SDL2/SDL_stdinc.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

struct Sprite {
	int x = 0;
	int w = 16;
	int h = 24;
	int frames = 1;
	int frame_ms = -1;
	Uint64 next = 0;
	int curr_frame = 0;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Sprite, x, w, h, frames,
												frame_ms);

class Entity {
	public:
	Position position;
	bool collision;
	Sprite sprite;
	Entity(Position pos, bool collision, Sprite sprite)
		: position{pos}, collision{collision}, sprite{sprite} {}
	virtual ~Entity() = default;
};

class Enemy : public Entity {
	public:
	std::string name;
	int maxhp;
	int attack;
	int defense;
	int hp;
	Enemy(Position pos, const nlohmann::json& j);
};

class Door : public Entity {
	public:
	static unsigned count;
	unsigned id;
	Door(Position pos) : Entity{pos, false, {64}}, id(count++) {}
};

class Item : public Entity {
	public:
	std::string name;
	int hp;
	int maxhp;
	int attack;
	int defense;
	enum Type {
		WEAPON,
		ARMOR,
		CONSUMABLE,
		NEUTRAL,
		INVALID = -1
	} type = INVALID;
	Item(Position pos, const nlohmann::json& j);
};
NLOHMANN_JSON_SERIALIZE_ENUM(Item::Type, {{Item::INVALID, nullptr},
										  {Item::WEAPON, "Weapon"},
										  {Item::ARMOR, "Armor"},
										  {Item::CONSUMABLE, "Consumable"},
										  {Item::NEUTRAL, "Neutral"}});

class Player : public Entity {
	public:
	int attack, defense, maxhp, hp;
	std::shared_ptr<Item> weapon, armor;
	std::vector<std::shared_ptr<Item>> bag;
	void use(std::shared_ptr<Item>);
	void add_stats(Item&);
	void subtract_stats(Item&);
	void add_hp(int);
	void add_maxHp(int);
	void add_attack(int);
	void add_defense(int);
	Player(const Sprite& sprite)
		: Entity{{0, 0}, true, sprite}, attack{1}, defense{1}, maxhp{5},
		  hp{maxhp} {}
};

#endif
