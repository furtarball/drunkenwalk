#include "include/game.h"
#include "include/config.h"
#include "include/map.h"
#include "include/renderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>
#include <typeinfo>
#include <vector>
using namespace std;

Config::Config(const char* json_path) {
	std::ifstream f{json_path};
	nlohmann::json j{nlohmann::json::parse(f)};
	j.at(0).get_to(*this);
}

void Game::run() {
	while (lvl() != -1) {
	}
}

bool Game::move_up() {
	Position newPos = player->position.up(level->map);
	player->sprite.x = 0;
	player->sprite.frames = 4;
	if (!level->collision(newPos)) {
		player->position = newPos;
		renderer.mvmtY = Animation{&Animation::linear, config.tile_h, 0,
								   round(renderer.fps.fps / 4.0)};
		return true;
	}
	return false;
}
bool Game::move_down() {
	Position newPos = player->position.down(level->map);
	player->sprite.x = 16;
	player->sprite.frames = 4;
	if (!level->collision(newPos)) {
		player->position = newPos;
		renderer.mvmtY = Animation{&Animation::linear, -config.tile_h, 0,
								   round(renderer.fps.fps / 4.0)};
		return true;
	}
	return false;
}
bool Game::move_left() {
	Position newPos = player->position.left(level->map);
	player->sprite.x = 32;
	player->sprite.frames = 4;
	if (!level->collision(newPos)) {
		player->position = newPos;
		renderer.mvmtX = Animation{&Animation::linear, config.tile_w, 0,
								   round(renderer.fps.fps / 6.0)};
		return true;
	}
	return false;
}
bool Game::move_right() {
	Position newPos = player->position.right(level->map);
	player->sprite.x = 48;
	player->sprite.frames = 4;
	if (!level->collision(newPos)) {
		player->position = newPos;
		renderer.mvmtX = Animation{&Animation::linear, -config.tile_w, 0,
								   round(renderer.fps.fps / 6.0)};
		return true;
	}
	return false;
}

bool Game::handle_movement(const Uint8* kbd) {
	bool moved = false;
	if (!(renderer.mvmtY ||
		  renderer.mvmtX)) { // if the character isn't already moving
		if (kbd[SDL_SCANCODE_UP] || kbd[SDL_SCANCODE_W])
			moved = move_up();
		else if (kbd[SDL_SCANCODE_DOWN] || kbd[SDL_SCANCODE_S])
			moved = move_down();
		else if (kbd[SDL_SCANCODE_LEFT] || kbd[SDL_SCANCODE_A])
			moved = move_left();
		else if (kbd[SDL_SCANCODE_RIGHT] || kbd[SDL_SCANCODE_D])
			moved = move_right();
	}
	return moved;
}

void Game::death() {
	SDL_Event e;
	do {
		SDL_GetWindowSize(renderer.window, &(config.window_w),
						  &(config.window_h));
		renderer.clear();
		SDL_Rect dst = {config.window_w / 2, config.window_h / 2, 0, 0};
		renderer.print("DEAD", Renderer::BOLD64, dst, 'c', 'c');
		renderer.present();
		SDL_WaitEvent(&e);
	} while (e.type != SDL_QUIT);
}

void Game::inventory() {
	SDL_Event e;
	size_t choice = 0;
	do {
		SDL_WaitEvent(&e);
		SDL_GetWindowSize(renderer.window, &(config.window_w),
						  &(config.window_h));
		renderer.clear();
		{
			stringstream text;
			text << "HP: " << player->hp << '/' << player->maxhp << endl;
			text << "Attack: " << player->attack << endl;
			text << "Defense: " << player->defense << endl << endl;
			SDL_Rect dst = {0, 0, 0, 0};
			renderer.print(text.str(), Renderer::REGULAR16, dst, 't', 'l');
		}
		if (e.key.type == SDL_KEYDOWN) {
			switch (e.key.keysym.scancode) {
			case SDL_SCANCODE_DOWN: {
				if ((choice + 1) < player->bag.size())
					choice++;
				break;
			}
			case SDL_SCANCODE_UP: {
				if (choice > 0)
					choice--;
				break;
			}
			case SDL_SCANCODE_SPACE: {
				if (choice < player->bag.size()) {
					auto item = player->bag[choice];
					if ((item->type == Item::WEAPON) ||
						(item->type == Item::ARMOR) ||
						(item->type == Item::CONSUMABLE)) {
						player->use(item);
						player->bag.erase(player->bag.begin() + choice);
					}
				}
				break;
			}
			default:
				break;
			}
		}
		{
			stringstream text;
			text << "Equipment:" << endl;
			if (player->weapon)
				text << player->weapon->name << endl;
			if (player->armor)
				text << player->armor->name << endl;
			text << "Bag:" << endl;
			for (size_t i = 0; i < player->bag.size(); i++) {
				if (i == choice)
					text << "> ";
				text << player->bag[i]->name << endl;
			}
			SDL_Rect dst = {config.window_w / 2, config.window_h / 2, 0, 0};
			renderer.print(text.str(), Renderer::REGULAR16, dst, 'c', 'c');
		}
		renderer.present();
	} while (!((e.key.type == SDL_KEYDOWN) &&
			   (e.key.keysym.scancode == SDL_SCANCODE_E)));
	renderer.fps.set_skip();
}

int Game::lvl() {
	// 0. random seed value, 1. door number, 2. map type
	static Seed seed = {0xf4bc0d54, 5};
	SDL_Event e;
	if (level)
		level->seed.generate(seed.begin(), seed.begin() + 1);
	level = std::make_unique<Level>(config.map_w, config.map_h, seed, player);
	renderer.camera.followPlayer();
	renderer.fade = Animation(&Animation::log, 255, 0, renderer.fps.fps / 2);
	bool door = false;
	while (true) {
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT:
				return -1;
			case SDL_KEYDOWN: {
				switch (e.key.keysym.scancode) {
				case SDL_SCANCODE_SPACE: {
					if (!(level->grab())) {
						level->fight();
						if (player->hp == 0) {
							death();
							SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
							return -1;
						}
					}
					break;
				}
				case SDL_SCANCODE_E: {
					inventory();
					SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);
					break;
				}
				default:
					break;
				}
				break;
			}
			}
		}
		bool moved{handle_movement(SDL_GetKeyboardState(NULL))};
		renderer.fps();
		SDL_GetWindowSize(renderer.window, &(config.window_w),
						  &(config.window_h));
		renderer.prepareAll(level->map, level->entities, player, seed);
		renderer.applyFade();
		renderer.present();

		if (moved) {
			if (!door) {
				for (auto i = level->entities.door0();
					 i < level->entities.door_end(); i++) {
					if (player->position == (*i)->position) {
						shared_ptr<Door> doorPtr =
							dynamic_pointer_cast<Door>(*i);
						seed[1] = doorPtr->id;
						door = true;
						renderer.fade = Animation(&Animation::linear, 0, 255,
												  renderer.fps.fps / 2);
						break;
					}
				}
			}
		}
		// if player has stepped through door and fadeout is complete
		if (door && (renderer.fade.funcType == nullptr))
			return 0;
	}
	return 0;
}
