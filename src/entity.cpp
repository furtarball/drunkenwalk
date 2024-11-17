#include "include/entity.h"
#include <memory>
#include <fstream>

Enemy::PossibleTypes Enemy::types;
unsigned Door::count = 0;
Item::PossibleTypes Item::types;

Enemy::PossibleTypes::PossibleTypes() {
  std::ifstream file("assets/enemies");
  do {
    Type t;
    file >> t.name;
    file >> t.offset;
    file >> t.frames;
    file >> t.maxHp;
    file >> t.attack;
    file >> t.defense;
    list.push_back(t);
  } while((!file.eof()) && (!file.fail()));
}
Item::PossibleTypes::PossibleTypes() {
  std::ifstream file("assets/items");
  do {
    Type t;
    file >> t.name;
    file >> t.maxHp;
    file >> t.hp;
    file >> t.attack;
    file >> t.defense;
    file >> t.category;
    size_t underscore = t.name.find('_');
    while(underscore != std::string::npos) {
      t.name[underscore] = ' ';
      underscore = t.name.find('_');
    }
    list.push_back(t);
  } while((!file.eof()) && (!file.fail()));
}

void Player::use(std::shared_ptr<Item> i) {
  add_stats(*i);
  if(i->category == Item::WEAPON) {
    if(weapon)
      subtract_stats(*weapon);
    weapon = i;
  }
  else if(i->category == Item::ARMOR) {
    if(armor)
      subtract_stats(*armor);
    armor = i;
  }
}

void Player::add_stats(Item& i) {
  add_maxHp(i.maxHp);
  add_hp(i.hp);
  add_attack(i.attack);
  add_defense(i.defense);
}

void Player::subtract_stats(Item& i) {
  add_maxHp(-(i.maxHp));
  add_attack(-(i.attack));
  add_defense(-(i.defense));
}

void Player::add_hp(int h) {
  hp += h;
  if(hp < 0)
    hp = 0;
  if(hp > maxHp)
    hp = maxHp;
}

void Player::add_maxHp(int m) {
  maxHp += m;
}

void Player::add_attack(int a) {
  attack += a;
}

void Player::add_defense(int d) {
  defense += d;
}
