#include "include/level.h"
#include "include/entity.h"
#include "include/mapgen.h"

void EntitiesArray::push_back(std::shared_ptr<Player> p) {
  if((size() > 0) && (typeid(*(at(0))) == typeid(Player)))
    at(0) = p;
  else
    insert(player(), std::shared_ptr<Entity>(p));
  players = 1;
}
void EntitiesArray::push_back(Enemy* e) {
  insert(mob_end(), std::shared_ptr<Entity>(e));
  mobs++;
}
void EntitiesArray::push_back(Door* d) {
  insert(door_end(), std::shared_ptr<Entity>(d));
  doors++;
}
void EntitiesArray::push_back(Item* i) {
  insert(item_end(), std::shared_ptr<Entity>(i));
  items++;
}
EntitiesArray::iterator EntitiesArray::erase(EntitiesArray::iterator pos) {
  size_t n = pos - begin();
  if((pos > mob0()) && (pos < mob_end()))
    mobs--;
  else if((pos > door0()) && (pos < door_end()))
    doors--;
  else if((pos > item0()) && (pos < item_end()))
    items--;
  std::vector<std::shared_ptr<Entity>>::erase(pos);
  return begin() + n;
}

Level::Level(std::array<unsigned, 2>& a, std::shared_ptr<Player>& p) : seed(a.begin(), a.end()), player(p) {
  switch(static_cast<MapType>(a[0] % static_cast<unsigned>(MAPTYPES))) {
  case CAVE_REGULAR:
    gen = std::unique_ptr<MapGenerator>(new DrunkenWalk(seed, map, entities));
    break;
  case CAVE_CORRIDOR:
    gen = std::unique_ptr<MapGenerator>(new TargetedDrunkenWalk(seed, map, entities));
    break;
  default:
    std::cerr << "No map generator created!" << std::endl;
  }
  entities.push_back(player);
  gen->generateMap();
  gen->populate();
}
bool Level::collision(const Position p) {
  if(map[p] != 1)
    return true;
  for(auto&& i : entities)
    if((i->position == p) && i->collisionAllowed)
      return true;
  return false;
}

bool Level::grab() {
  for(auto i = entities.item0(); i < entities.item_end(); i++) {
    std::shared_ptr<Item> itemPtr = std::dynamic_pointer_cast<Item>(*i);
    if(itemPtr->position == player->position) {
      player->bag.push_back(itemPtr);
      entities.erase(i);
      return true;
    }
  }
  return false;
}

void Level::fight() {
  for(int i = -1; i <= 1; i++) {
    for(int j = -1; j <= 1; j++) {
      Position p(player->position.getX() + j, player->position.getY() + i);
      for(auto k = entities.mob0(); k < entities.mob_end(); k++) {
	if((*k)->position == p) {
	  std::shared_ptr<Enemy> enemy = std::dynamic_pointer_cast<Enemy>(*k);
	  unsigned damage = std::lround((player->attack * 100.0) / static_cast<double>(enemy->defense + 100));
	  if(enemy->hp > damage)
	    enemy->hp -= damage;
	  else
	    enemy->hp = 0;
	  if(enemy->hp == 0) {
	    entities.erase(k);
	  }
	  else {
	    damage = std::lround((enemy->attack * 100.0) / static_cast<double>(player->defense + 100));
	    if(player->hp > damage)
	      player->hp -= damage;
	    else
	      player->hp = 0;
	  }
	}
      }
    }
  }
}
