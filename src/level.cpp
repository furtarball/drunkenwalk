#include "include/level.h"
#include "include/entity.h"

Level::Level(std::array<unsigned, 2>& a, std::shared_ptr<Player>& p) : seed(a.begin(), a.end()), rng(seed), player(p) {
  generateMap();
  //map.exportPGM("map.pgm");
  populate();
  
}

void Level::drunkenWalk() {
  // array of pointers to member functions of class Position
  Position (Position::*movements[])() = { &Position::up, &Position::down,
				      &Position::left, &Position::right };
  Position p = map.getStart();
  for(size_t i = 0; i < STEPS; i++) {
    map[p.getX()][p.getY()] = 1;
    unsigned dir = rng() & 3;
    p = (p.*(movements[dir]))();
  }
}

Position Level::randomizePosition() {
  Position p;
  bool withinBorders, occupied;
  do { // picks random position until a valid and unoccupied one is found
    p = Position((rng() % (Map::X - 3)) + 1, (rng() % (Map::Y - 3)) + 1);
    withinBorders = (map[p.getX()][p.getY()] == 1);
    occupied = false;
    if(withinBorders)
      for(auto&& i : entities) {
	if(p == i->position) {
	  occupied = true;
	  break;
	}
      }
  } while(!withinBorders || occupied);
  return p;
}

void Level::populate() {
  //pick random positions until an unoccupied one is found
  //constuct an object on that position
  //rinse and repeat
  player->position = randomizePosition();
  entities.push_back(player);
  nMobs = map.countWalkable() / ((rng() % 100) + 50);
  for(size_t i = 0; i < nMobs; i++)
    entities.push_back(std::shared_ptr<Entity>(new Enemy(randomizePosition(), rng() % Enemy::types.list.size())));
  nDoors = (nMobs / 4) + 1;
  Door::count = 0;
  for(size_t i = 0; i < nDoors; i++)
    entities.push_back(std::shared_ptr<Entity>(new Door(randomizePosition())));
  nItems = nDoors;
  for(size_t i = 0; i < nItems; i++)
    entities.push_back(std::shared_ptr<Entity>(new Item(randomizePosition(), rng() % Item::types.list.size())));
}

void Level::generateMap() {
  map.zero();
  drunkenWalk();
  map.smoothen(1, 8);
  map.smoothen(0, 8);
  map.smoothen(0, 3);
  map.drawWall();
  map.zeroBorder();
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
