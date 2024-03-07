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
  size_t offset, frames;

  Entity(Position p, bool cd, size_t o, size_t f) : position(p), collisionAllowed(cd), offset(o), frames(f) {}
  virtual ~Entity() = default;
};

class Enemy : public Entity {
 public:
  static struct PossibleTypes {
    struct Type {
      std::string name;
      unsigned offset, frames, maxHp, attack, defense;
    };
    std::vector<Type> list;
    PossibleTypes();
  } types;
  std::string name;
  unsigned maxHp, attack, defense, hp;
  Enemy(Position p, unsigned n) : Entity(p, true, types.list[n].offset, types.list[n].frames), name(types.list[n].name), maxHp(types.list[n].maxHp), attack(types.list[n].attack), defense(types.list[n].defense), hp(types.list[n].maxHp) {}
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
      unsigned maxHp, hp, attack, defense, category;
    };
    std::vector<Type> list;
    PossibleTypes();
  } types;
  std::string name;
  unsigned maxHp, hp, attack, defense;
  Item(Position p, unsigned n) : Entity(p, false, Enemy::types.list.size() + 3, 1), name(types.list[n].name), maxHp(types.list[n].maxHp), hp(types.list[n].hp), attack(types.list[n].attack), defense(types.list[n].defense) {}
};

class Player : public Entity {
  public:
  unsigned attack, defense, maxHp, hp;
  std::shared_ptr<Item> weapon, armor;
  std::vector<std::shared_ptr<Item>> bag; 
  Player() : Entity(Position(0, 0), true, 1, 1), attack(1), defense(1), maxHp(5), hp(5) {}
};

#endif
