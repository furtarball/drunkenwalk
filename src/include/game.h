#ifndef GAME_H
#define GAME_H

#include "config.h"
#include "entity.h"
#include "level.h"
#include "map.h"
#include "renderer.h"
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
	void stop_walking();
	bool move_up();
	bool move_down();
	bool move_left();
	bool move_right();
	int lvl(); // returns 0 to generate another level, -1 to exit
	void inventory();
	void death();
	bool handle_movement(const Uint8*);

	public:
	Game(const char* config_file)
		: config{config_file},
		  player{std::make_shared<Player>(config.player_sprite_object)},
		  renderer{config, player->position} {}
	void run();
};

#endif
