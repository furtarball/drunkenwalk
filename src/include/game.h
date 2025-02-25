#ifndef GAME_H
#define GAME_H

#include "entity.h"
#include "level.h"
#include "map.h"
#include "renderer.h"
#include "config.h"
#include <SDL2/SDL.h>
#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

class Game {
	Config config;
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
	Game() : config{"assets/config.json"}, player(new Player), renderer{config} {}
	void run();
};

#endif
