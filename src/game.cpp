#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "include/game.h"
#include "include/map.h"
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
  Screen s = LEVEL;
  while(s != QUIT) {
    switch(s) {
    case LEVEL:
      lvl(s);
      break;
    case DEATH:
      death(s);
      break;
    default:
      break;
    }
  }
}

void Game::handleKbdEvents(const Uint8* kbd, SDL_Event& e) {
  SDL_PollEvent(&e);
  if(!(renderer.mvmtY || renderer.mvmtX)) { // if the character isn't already moving
    if(kbd[SDL_SCANCODE_UP] || kbd[SDL_SCANCODE_W]) {
      Position newPos = player->position.up();
      if(!level->collision(newPos)) {
	player->position = newPos;
	renderer.mvmtY = Animation(&Animation::linear, spriteY, 0, round(renderer.fps.fps / 4.0));
      }
    }
    else if(kbd[SDL_SCANCODE_DOWN] || kbd[SDL_SCANCODE_S]) {
      Position newPos = player->position.down();
      if(!level->collision(newPos)) {
	player->position = newPos;
	renderer.mvmtY = Animation(&Animation::linear, -spriteY, 0, round(renderer.fps.fps / 4.0));
      }
    }
    else if(kbd[SDL_SCANCODE_LEFT] || kbd[SDL_SCANCODE_A]) {
      Position newPos = player->position.left();
      if(!level->collision(newPos)) {
	player->position = newPos;
	renderer.mvmtX = Animation(&Animation::linear, spriteX, 0, round(renderer.fps.fps / 6.0));
      }
    }
    else if(kbd[SDL_SCANCODE_RIGHT] || kbd[SDL_SCANCODE_D]) {
      Position newPos = player->position.right();
      if(!level->collision(newPos)) {
	player->position = newPos;
	renderer.mvmtX = Animation(&Animation::linear, -spriteX, 0, round(renderer.fps.fps / 6.0));
      }
    }
  }
  if(e.key.type == SDL_KEYDOWN) {
    if(e.key.keysym.scancode == SDL_SCANCODE_SPACE) {
      level->grab();
      level->fight();
    }
  }
}

void Game::death(Screen& s) {
  SDL_Event e;
  renderer.clear();
  SDL_Rect dst = { spriteX * windowX, spriteY * windowY, 0, 0 };
  renderer.print("DEAD", Renderer::BOLD64, dst, 'c', 'c');
  renderer.present();
  do
    SDL_WaitEvent(&e);
  while(e.type != SDL_QUIT);
  s = QUIT;
}

void Game::lvl(Screen& s) {
  static array<unsigned, 2> seed = { 0xf4bc0d54, 5 };
  SDL_Event e;
  if(level)
    level->seed.generate(seed.begin(), seed.begin() + 1);
  level.reset(new Level(seed, player));
  renderer.fade = Animation(&Animation::log, 255, 0, renderer.fps.fps / 2);
  bool door = false;
  while(true) {
    handleKbdEvents(SDL_GetKeyboardState(NULL), e);
    if(e.type == SDL_QUIT) {
      s = QUIT;
      break;
    }
    if(player->hp == 0) {
      s = DEATH;
      break;
    }
    renderer.fps();
    SDL_GetWindowSize(renderer.window, &renderer.windowX, &renderer.windowY);
    renderer.prepareAll(level->map, level->entities, player, seed);
    renderer.applyFade();
    renderer.present();
    
    if(!door) {
	for(size_t i = level->nMobs + 1; i < level->nMobs + level->nDoors + 1; i++) {
	  if(player->position == level->entities[i]->position) {
	    shared_ptr<Door> doorPtr = dynamic_pointer_cast<Door>(level->entities[i]);
	    seed[1] = doorPtr->id;
	    door = true;
	    renderer.fade = Animation(&Animation::linear, 0, 255, renderer.fps.fps / 2);
	    break;
	  }
	}
    }
    // if player has stepped through door and fadeout is complete
    else if(renderer.fade.funcType == nullptr)
      break;
  }
}
