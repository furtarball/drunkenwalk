#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <array>
#include <random>
#include <vector>
#include <iostream>
#include <cmath>
#include "map.h"
#include "level.h"
#include "entity.h"
#include "renderer.h"

class Game {
  std::unique_ptr<Level> level;
  std::shared_ptr<Player> player;
  Renderer renderer;
  bool move_up();
  bool move_down();
  bool move_left();
  bool move_right();
  int lvl(); // returns 0 to generate another level, -1 to exit
  void inventory();
  void death();
  bool handle_movement(const Uint8*, SDL_Event&);
public:
  Game() : player(new Player) {}
  void run();
};

#endif
