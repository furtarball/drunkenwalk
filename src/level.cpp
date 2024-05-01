#include "include/level.h"
#include "include/entity.h"
#include "include/mapgen.h"

Level::Level(std::array<unsigned, 2>& a, std::shared_ptr<Player>& p) : seed(a.begin(), a.end()), player(p) {
  switch(static_cast<MapType>(a[0] % static_cast<unsigned>(MAPTYPES))) {
  case CAVE_REGULAR:
    gen = std::unique_ptr<MapGenerator>(new DrunkenWalk(seed, map, entities, nDoors, nMobs, nItems));
    break;
  case CAVE_CORRIDOR:
    gen = std::unique_ptr<MapGenerator>(new TargetedDrunkenWalk(seed, map, entities, nDoors, nMobs, nItems));
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

void Level::grab() {
  for(size_t i = nMobs + nDoors + 1; i < nMobs + nDoors + nItems + 1; i++) {
    std::shared_ptr<Item> itemPtr = std::dynamic_pointer_cast<Item>(entities[i]);
    if(itemPtr->position == player->position) {
      player->bag.push_back(itemPtr);
      entities.erase(entities.begin() + i);
      nItems--;
      break;
    }
  }
}

void Level::fight() {
  for(int i = -1; i <= 1; i++) {
    for(int j = -1; j <= 1; j++) {
      Position p(player->position.getX() + j, player->position.getY() + i);
      for(size_t k = 1; k < nMobs + 1; k++) {
	if(entities[k]->position == p) {
	  std::shared_ptr<Enemy> enemyPtr = std::dynamic_pointer_cast<Enemy>(entities[k]);
	  unsigned damage = std::lround((static_cast<double>(player->attack * 100) / (enemyPtr->defense + 100)));
	  if(enemyPtr->hp > damage)
	    enemyPtr->hp -= damage;
	  else
	    enemyPtr->hp = 0;
	  if(enemyPtr->hp == 0) {
	    entities.erase(entities.begin() + k);
	    nMobs--;
	  }
	  else {
	    damage = std::lround((static_cast<double>(enemyPtr->attack * 100) / (player->defense + 100)));
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
