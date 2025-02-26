#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include <SDL2/SDL_stdinc.h>
#include "map.h"

class Entity {
	public:
	Position position;
	bool collision;
	struct Sprite {
		int pos_x = 0;
		int dim_x = 16;
		int dim_y = 24;
		int frames = 1;
		int frame_ms = -1;
		Uint64 next = 0;
		int curr_frame = 0;
	} sprite;
	Entity(Position pos, bool collision, Sprite sprite)
		: position{pos}, collision{collision}, sprite{sprite} {}
	virtual ~Entity() = default;
};
void from_json(const nlohmann::json& j, Entity::Sprite& s);

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
  Door(Position pos) : Entity{pos, false, {80}}, id(count++) {}
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
  Player() : Entity{{0, 0}, true, {16}}, attack{1}, defense{1}, maxhp{5}, hp{maxhp} {}
};

#endif
