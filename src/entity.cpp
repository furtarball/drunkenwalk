#include "include/entity.h"
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
    file >> t.category; // weapon, armor, edible, neutral
    size_t underscore = t.name.find('_');
    while(underscore != std::string::npos) {
      t.name[underscore] = ' ';
      underscore = t.name.find('_');
    }
    list.push_back(t);
  } while((!file.eof()) && (!file.fail()));
}
