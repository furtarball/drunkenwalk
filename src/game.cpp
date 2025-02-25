#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "include/game.h"
#include "include/map.h"
#include "include/renderer.h"
#include <array>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <typeinfo>
using namespace std;

void Game::run() {
  while(lvl() != -1) {}
}

bool Game::move_up() {
  Position newPos = player->position.up();
  if(!level->collision(newPos)) {
    player->position = newPos;
    renderer.mvmtY = Animation(&Animation::linear, spriteY, 0, round(renderer.fps.fps / 4.0));
    return true;
  }
  return false;
}
bool Game::move_down() {
  Position newPos = player->position.down();
  if(!level->collision(newPos)) {
    player->position = newPos;
    renderer.mvmtY = Animation(&Animation::linear, -spriteY, 0, round(renderer.fps.fps / 4.0));
    return true;
  }
  return false;
}
bool Game::move_left() {
  Position newPos = player->position.left();
  if(!level->collision(newPos)) {
    player->position = newPos;
    renderer.mvmtX = Animation(&Animation::linear, spriteX, 0, round(renderer.fps.fps / 6.0));
    return true;
  }
  return false;
}
bool Game::move_right() {
  Position newPos = player->position.right();
  if(!level->collision(newPos)) {
    player->position = newPos;
    renderer.mvmtX = Animation(&Animation::linear, -spriteX, 0, round(renderer.fps.fps / 6.0));
    return true;
  }
  return false;
}

bool Game::handle_movement(const Uint8* kbd, SDL_Event& e) {
  bool moved = false;
  if(!(renderer.mvmtY || renderer.mvmtX)) { // if the character isn't already moving
    if(kbd[SDL_SCANCODE_UP] || kbd[SDL_SCANCODE_W])
      moved = move_up();
    else if(kbd[SDL_SCANCODE_DOWN] || kbd[SDL_SCANCODE_S])
      moved = move_down();
    else if(kbd[SDL_SCANCODE_LEFT] || kbd[SDL_SCANCODE_A])
      moved = move_left();
    else if(kbd[SDL_SCANCODE_RIGHT] || kbd[SDL_SCANCODE_D])
      moved = move_right();
  }
  
  if(e.key.type == SDL_KEYDOWN) {
    if(e.key.keysym.scancode == SDL_SCANCODE_SPACE) {
      if(!(level->grab()))
	level->fight();
    }
  }
  return moved;
}

void Game::death() {
  SDL_Event e;
  do {
    SDL_PollEvent(&e);
    SDL_GetWindowSize(renderer.window, &renderer.windowX, &renderer.windowY);
    renderer.clear();
    SDL_Rect dst = { renderer.windowX / 2, renderer.windowY / 2, 0, 0 };
    renderer.print("DEAD", Renderer::BOLD64, dst, 'c', 'c');
    renderer.present();
  } while(e.type != SDL_QUIT);
}

void Game::inventory() {
  SDL_Event e;
  size_t choice = 0;
  do {
    SDL_WaitEvent(&e);
    SDL_GetWindowSize(renderer.window, &renderer.windowX, &renderer.windowY);
    renderer.clear();
    {
      stringstream text;
      text << "HP: " << player->hp << '/' << player->maxhp << endl;
      text << "Attack: " << player->attack << endl;
      text << "Defense: " << player->defense << endl << endl;
      SDL_Rect dst = { 0, 0, 0, 0 };
      renderer.print(text.str(), Renderer::REGULAR16, dst, 't', 'l');
    }
    if(e.key.type == SDL_KEYDOWN) {
      if(e.key.keysym.scancode == SDL_SCANCODE_DOWN) {
	if((choice + 1) < player->bag.size()) choice++;
      }
      if(e.key.keysym.scancode == SDL_SCANCODE_UP) {
	if(choice > 0) choice--;
      }
      if(e.key.keysym.scancode == SDL_SCANCODE_SPACE) {
	if(choice < player->bag.size()) {
	  auto item = player->bag[choice];
	  if((item->type == Item::WEAPON) ||
	     (item->type == Item::ARMOR) ||
	     (item->type == Item::CONSUMABLE)) {
	    player->use(item);
	    player->bag.erase(player->bag.begin() + choice);
	  }
	}
      }
    }
    {
      stringstream text;
      text << "Equipment:" << endl;
      if(player->weapon)
	text << player->weapon->name << endl;
      if(player->armor)
	text << player->armor->name << endl;
      text << "Bag:" << endl;
      for(size_t i = 0; i < player->bag.size(); i++) {
	if(i == choice)
	  text << "> ";
	text << player->bag[i]->name << endl;
      }
      SDL_Rect dst = { renderer.windowX / 2, renderer.windowY / 2, 0, 0 };
      renderer.print(text.str(), Renderer::REGULAR16, dst, 'c', 'c');
    }
    renderer.present();
  } while(!((e.key.type == SDL_KEYDOWN) && (e.key.keysym.scancode == SDL_SCANCODE_E)));
  renderer.fps.set_skip();
}

int Game::lvl() {
  // 0. random seed value, 1. door number, 2. map type
  static Seed seed = { 0xf4bc0d54, 5 };
  SDL_Event e;
  if(level)
    level->seed.generate(seed.begin(), seed.begin() + 1);
  level.reset(new Level(seed, player));
  renderer.fade = Animation(&Animation::log, 255, 0, renderer.fps.fps / 2);
  bool door = false;
  while(true) {
    SDL_PollEvent(&e);
    bool moved = handle_movement(SDL_GetKeyboardState(NULL), e);
    if(e.type == SDL_QUIT)
      return -1;
    if(player->hp == 0) {
      death();
      return -1;
    }
    if((e.key.type == SDL_KEYDOWN) && (e.key.keysym.scancode == SDL_SCANCODE_E))
      inventory();
    renderer.fps();
    SDL_GetWindowSize(renderer.window, &renderer.windowX, &renderer.windowY);
    renderer.clear();
    renderer.prepareAll(level->map, level->entities, player, seed);
    renderer.applyFade();
    renderer.present();

    if(moved) {
      if(!door) {
	for(auto i = level->entities.door0(); i < level->entities.door_end(); i++) {
	  if(player->position == (*i)->position) {
	    shared_ptr<Door> doorPtr = dynamic_pointer_cast<Door>(*i);
	    seed[1] = doorPtr->id;
	    door = true;
	    renderer.fade = Animation(&Animation::linear, 0, 255, renderer.fps.fps / 2);
	    break;
	  }
	}
      }
    }
    // if player has stepped through door and fadeout is complete
    if(door && (renderer.fade.funcType == nullptr))
      return 0;
  }
  return 0;
}
