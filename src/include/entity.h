#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <string>
#include <memory>
#include "map.h"

class Entity {
 public:
  Position position;
  bool collisionAllowed;
  int offset, frames;

  Entity(Position p, bool cd, size_t o, size_t f) : position(p), collisionAllowed(cd), offset(o), frames(f) {}
  virtual ~Entity() = default;
};

class Enemy : public Entity {
 public:
  
  static struct PossibleTypes {
    struct Type {
      std::string name;
      int offset, frames,
	maxHp, attack, defense;
    };
    std::vector<Type> list;
    PossibleTypes();
  } types;

  
  std::string name;
  int maxHp, attack, defense, hp;
  Enemy(Position p, unsigned n) :
    Entity(p, true, types.list[n].offset, types.list[n].frames),
    name(types.list[n].name), maxHp(types.list[n].maxHp),
    attack(types.list[n].attack), defense(types.list[n].defense),
    hp(types.list[n].maxHp) {}
};

class Door : public Entity {
 public:
  static unsigned count;
  unsigned id;
  Door(Position p) : Entity(p, false, Enemy::types.list.size() + 2, 1), id(count++) {}
};

class Item : public Entity {
 public:
  
  static struct PossibleTypes {
    struct Type {
      std::string name;
      int maxHp, hp, attack,
	defense, category;
    };
    std::vector<Type> list;
    PossibleTypes();
  } types;

  
  std::string name;
  int maxHp, hp, attack, defense;
  enum Cat { WEAPON, ARMOR, CONSUMABLE, NEUTRAL, CATEGORIES } category;
  Item(Position p, unsigned n) :
    Entity(p, false, Enemy::types.list.size() + 3, 1),
    name(types.list[n].name), maxHp(types.list[n].maxHp),
    hp(types.list[n].hp), attack(types.list[n].attack),
    defense(types.list[n].defense), category(static_cast<Cat>(types.list[n].category)) {}
};

class Player : public Entity {
  public:
  int attack, defense, maxHp, hp;
  std::shared_ptr<Item> weapon, armor;
  std::vector<std::shared_ptr<Item>> bag;
  void use(std::shared_ptr<Item>);
  void add_stats(Item&);
  void subtract_stats(Item&);
  void add_hp(int);
  void add_maxHp(int);
  void add_attack(int);
  void add_defense(int);
  Player() : Entity(Position(0, 0), true, 1, 1), attack(1), defense(1), maxHp(5), hp(5) {}
};

#endif
