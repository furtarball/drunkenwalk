#include "include/renderer.h"
#include "include/map.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <typeinfo>
#include <vector>

Renderer::SDLGuard::SDLGuard() {
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw std::runtime_error{SDL_GetError()};
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		SDL_Quit();
		throw std::runtime_error{SDL_GetError()};
	}
	if (TTF_Init() != 0) {
		IMG_Quit();
		SDL_Quit();
		throw std::runtime_error{SDL_GetError()};
	}
}
Renderer::SDLGuard::~SDLGuard() {
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

Renderer::Renderer(const Config& config, const Position& player_pos)
	: cfg{config}, window{SDL_CreateWindow("Drunken Walk",
					   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
					   cfg.window_w, cfg.window_h, SDL_WINDOW_RESIZABLE)},
	  renderer{SDL_CreateRenderer(
		  window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)},
	  mapLayer{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
		  SDL_TEXTUREACCESS_TARGET, cfg.map_w * cfg.tile_w,
		  cfg.map_h * cfg.tile_h)},
	  entityLayer{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
		  SDL_TEXTUREACCESS_TARGET, cfg.map_w * cfg.tile_w,
		  cfg.map_h * cfg.tile_h)},
	  environment{
		  IMG_LoadTexture(renderer, cfg.asset_path(cfg.environment).c_str())},
	  entities{IMG_LoadTexture(renderer, cfg.asset_path(cfg.entities).c_str())},
	  player_spritesheet{IMG_LoadTexture(
		  renderer, cfg.asset_path(cfg.player_spritesheet).c_str())},
	  fonts{TTF_OpenFont(cfg.asset_path(cfg.font_regular_file).c_str(),
				cfg.font_regular_size),
		  TTF_OpenFont(cfg.asset_path(cfg.font_medium_file).c_str(),
			  cfg.font_medium_size),
		  TTF_OpenFont(
			  cfg.asset_path(cfg.font_big_file).c_str(), cfg.font_big_size)},
	  fps{60}, fade{&Animation::square, 255, 0, fps.fps},
	  camera{cfg, mvmtX, mvmtY, player_pos} {
	SDL_SetTextureBlendMode(mapLayer, SDL_BLENDMODE_BLEND);
	SDL_SetTextureBlendMode(entityLayer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void Renderer::clear() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
}

void Renderer::renderMapLayer(Map& map) {
	auto& bg = cfg.environment_background;
	SDL_SetRenderDrawColor(renderer, bg[0], bg[1], bg[2], bg[3]);
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, mapLayer);
	SDL_Rect src{0, 0, cfg.tile_w, cfg.tile_h};
	SDL_Rect dst{src};
	for (int i = 0; i < cfg.map_h; i++) {
		for (int j = 0; j < cfg.map_w; j++) {
			if (map[j][i] > 0) {
				src.x = (map[j][i] - 1) * cfg.tile_w;
				SDL_RenderCopy(renderer, environment, &src, &dst);
			}
			dst.x += cfg.tile_w;
		}
		dst.x = 0;
		dst.y += cfg.tile_h;
	}
	SDL_SetRenderTarget(renderer, NULL);
}

void Renderer::drawEntities(EntitiesArray& earr) {
	SDL_SetRenderTarget(renderer, entityLayer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	auto& bg = cfg.environment_background;
	SDL_SetRenderDrawColor(renderer, bg[0], bg[1], bg[2], bg[3]);
	for (auto i : earr) {
		int x = i->position.getX(), y = i->position.getY();
		if (camera.visible(i->position)) {
			auto& s = i->sprite;
			if ((s.frames > 1) && (fps.ticks >= s.next)) {
				auto advance{(fps.ticks - s.next) / s.frame_ms};
				s.curr_frame = (s.curr_frame + advance + 1) % s.frames;
				s.next = fps.ticks + s.frame_ms;
			}

			SDL_Rect offset{s.x, (s.curr_frame) * s.h, s.w, s.h};
			SDL_Rect pos{((x * cfg.tile_w) + (cfg.tile_w / 2)) - (s.w / 2),
				(y * cfg.tile_h) - (s.h - cfg.tile_h), s.w, s.h};
			if (i == *(earr.player())) {
				pos.x += mvmtX();
				pos.y += mvmtY();
				if ((!mvmtX) && (!mvmtY) &&
					((s.curr_frame == 0) || (s.curr_frame == 2)))
					s.frames = 1;
				SDL_RenderCopy(renderer, player_spritesheet, &offset, &pos);
			} else
				SDL_RenderCopy(renderer, entities, &offset, &pos);
		}
	}
	SDL_SetRenderTarget(renderer, NULL);
}

void Renderer::prepareAll(
	Map& map, EntitiesArray& earr, std::shared_ptr<Player> player, Seed& seed) {
	renderMapLayer(map);
	drawEntities(earr);
	camera.followPlayer();
	SDL_Rect targetRect{.x{(cfg.window_w - (camera.sdl().w * cfg.scale)) / 2},
		.y{(cfg.window_h - (camera.sdl().h * cfg.scale)) / 2},
		.w{camera.sdl().w * cfg.scale},
		.h{camera.sdl().h * cfg.scale}};
	SDL_RenderCopy(renderer, mapLayer, camera, &targetRect);
	SDL_RenderCopy(renderer, entityLayer, camera, &targetRect);
	drawOSD(player, seed);
}

void Renderer::applyFade() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, fade());
	SDL_RenderFillRect(renderer, NULL);
}

unsigned Animation::operator()() {
	if (funcType == nullptr)
		return finalValue;
	currentValue = (this->*(this->funcType))();
	currentFrame++;
	if (currentValue == finalValue)
		funcType = nullptr;
	return currentValue;
}

void FrameRate::measure() {
	static Uint64 prevTicks = ticks;
	static unsigned frames = 0;
	if (skip) {
		prevTicks = ticks;
		frames = 0;
		skip = false;
		return;
	}
	if (ticks - prevTicks >= 1000) {
		fps = frames;
		frames = 0;
		prevTicks = ticks;
	}
	frames++;
}

bool Camera::visible(Position p) {
	if ((((p.getX() + 1) * cfg.tile_w) < rect.x) ||
		(((p.getY() + 1) * cfg.tile_h) < rect.y) ||
		(((p.getX() - 1) * cfg.tile_w) > (rect.x + rect.w)) ||
		(((p.getY() - 1) * cfg.tile_h) > (rect.y + rect.h)))
		return false;
	return true;
}

void Camera::followPlayer() {
	rect.w = std::min<int>(cfg.map_w * cfg.tile_w, cfg.window_w / cfg.scale);
	rect.h = std::min<int>(cfg.map_h * cfg.tile_h, cfg.window_h / cfg.scale);

	rect.x =
		((player_pos.getX() * cfg.tile_w) +		// player's position
			(cfg.player_sprite_object.w / 2) -	// exact middle of player sprite
			(cfg.window_w / (cfg.scale * 2))) + // middle of window
		mvmtX.currentValue;
	if (rect.x < 0)
		rect.x = 0;
	else if ((rect.x + rect.w) > (cfg.map_w * cfg.tile_w))
		rect.x = (cfg.map_w * cfg.tile_w) - rect.w;

	rect.y =
		((player_pos.getY() * cfg.tile_h) +		// player's position
			(cfg.player_sprite_object.h / 2) -	// exact middle of player sprite
			(cfg.window_h / (cfg.scale * 2))) + // middle of window
		mvmtY.currentValue;
	if (rect.y < 0)
		rect.y = 0;
	else if ((rect.y + rect.h) > (cfg.map_h * cfg.tile_h))
		rect.y = (cfg.map_h * cfg.tile_h) - rect.h;
}
