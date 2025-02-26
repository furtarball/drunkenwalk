#include "include/renderer.h"
#include "include/map.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
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

Renderer::Renderer(const Config& config)
	: cfg{config},
	  window{SDL_CreateWindow("Drunken Walk", SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED, cfg.window_w,
							  cfg.window_h, SDL_WINDOW_RESIZABLE)},
	  renderer{SDL_CreateRenderer(
		  window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)},
	  mapLayer{SDL_CreateTexture(
		  renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET,
		  cfg.map_w * cfg.tile_w, cfg.map_h * cfg.tile_h)},
	  entityLayer{SDL_CreateTexture(
		  renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
		  cfg.map_w * cfg.tile_w, cfg.map_h * cfg.tile_h)},
	  environment{
		  IMG_LoadTexture(renderer, cfg.asset_path(cfg.environment).c_str())},
	  entities{IMG_LoadTexture(renderer, cfg.asset_path(cfg.entities).c_str())},
	  fonts{TTF_OpenFont(cfg.asset_path(cfg.font_regular_file).c_str(),
						 cfg.font_regular_size),
			TTF_OpenFont(cfg.asset_path(cfg.font_medium_file).c_str(),
						 cfg.font_medium_size),
			TTF_OpenFont(cfg.asset_path(cfg.font_big_file).c_str(),
						 cfg.font_big_size)},
	  fps{60}, fade{&Animation::square, 255, 0, fps.fps}, camera{cfg} {

	SDL_SetTextureBlendMode(entityLayer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void Renderer::renderMapLayer(Map& map) {
	SDL_SetRenderTarget(renderer, mapLayer);
	SDL_Rect src = {0, 0, cfg.tile_w, cfg.tile_h};
	SDL_Rect dst = src;
	for (int i = 0; i < cfg.map_h; i++) {
		for (int j = 0; j < cfg.map_w; j++) {
			src.x = map[j][i] * cfg.tile_w;
			SDL_RenderCopy(renderer, environment, &src, &dst);
			dst.x += cfg.tile_w;
		}
		dst.x = 0;
		dst.y += cfg.tile_h;
	}
	SDL_SetRenderTarget(renderer, NULL);
}

void Renderer::drawEntities(EntitiesArray& earr) {
	SDL_SetRenderTarget(renderer, entityLayer);
	SDL_RenderClear(renderer);
	for (auto i : earr) {
		int x = i->position.getX(), y = i->position.getY();
		if (camera.visible(i->position)) {
			auto& s = i->sprite;
			if ((s.frames > 1) && (fps.ticks >= s.next)) {
				auto advance{(fps.ticks - s.next) / s.frame_ms};
				s.curr_frame = (s.curr_frame + advance + 1) % s.frames;
				s.next = fps.ticks + s.frame_ms;
			}

			SDL_Rect offset{s.pos_x, (s.curr_frame) * s.dim_y, s.dim_x,
							s.dim_y};
			SDL_Rect pos{((x * cfg.tile_w) + (cfg.tile_w / 2)) - (s.dim_x / 2),
						 (y * cfg.tile_h) - (s.dim_y - cfg.tile_h), s.dim_x,
						 s.dim_y};
			if (i == *(earr.player())) {
				pos.x += mvmtX();
				pos.y += mvmtY();
			}
			SDL_RenderCopy(renderer, entities, &offset, &pos);
		}
	}
	SDL_SetRenderTarget(renderer, NULL);
}

void Renderer::prepareAll(Map& map, EntitiesArray& earr,
						  std::shared_ptr<Player> player, Seed& seed) {
	renderMapLayer(map);
	SDL_Rect targetRect;
	targetRect.w = static_cast<int>(ceil(camera.sdl.w * cfg.scale));
	targetRect.h = static_cast<int>(ceil(camera.sdl.h * cfg.scale));
	targetRect.x = static_cast<int>(ceil((cfg.window_w - targetRect.w) / 2.0));
	targetRect.y = static_cast<int>(ceil((cfg.window_h - targetRect.h) / 2.0));
	SDL_RenderCopy(renderer, mapLayer, &camera.sdl, &targetRect);
	drawEntities(earr);
	camera.followPlayer(player->position, mvmtX, mvmtY);
	SDL_RenderCopy(renderer, entityLayer, &camera.sdl, &targetRect);
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
	if ((((p.getX() + 1) * cfg.tile_w) < sdl.x) ||
		(((p.getY() + 1) * cfg.tile_h) < sdl.y) ||
		(((p.getX() - 1) * cfg.tile_w) > (sdl.x + sdl.w)) ||
		(((p.getY() - 1) * cfg.tile_h) > (sdl.y + sdl.h)))
		return false;
	return true;
}

void Camera::followPlayer(Position& pos, Animation& mvmtX, Animation& mvmtY) {
	sdl = {static_cast<int>(pos.getX() * cfg.tile_w -
							(cfg.window_w / (cfg.scale * 2))),
		   static_cast<int>(pos.getY() * cfg.tile_h -
							(cfg.window_h / (cfg.scale * 2))),
		   static_cast<int>(ceil(cfg.window_w / cfg.scale)),
		   static_cast<int>(ceil(cfg.window_h / cfg.scale))};
	sdl.x += mvmtX.currentValue;
	sdl.y += mvmtY.currentValue;
	// check if the map is narrower (shorter) than the viewport
	// if not, check if the above has tried to move it outside the map
	if (((cfg.map_w - 1) * cfg.tile_w) < sdl.w) {
		sdl.w = ((cfg.map_w - 1) * cfg.tile_w);
		sdl.x = 0;
	} else {
		if (sdl.x < cfg.tile_w)
			sdl.x = cfg.tile_w;
		else if ((sdl.x + sdl.w) > ((cfg.map_w - 1) * cfg.tile_w))
			sdl.x = (cfg.map_w - 1) * cfg.tile_w - sdl.w;
	}
	if (((cfg.map_h - 1) * cfg.tile_h) < sdl.h) {
		sdl.h = ((cfg.map_h - 1) * cfg.tile_h);
		sdl.y = 0;
	} else {
		if (sdl.y < cfg.tile_h)
			sdl.y = cfg.tile_h;
		else if ((sdl.y + sdl.h) > ((cfg.map_h - 1) * cfg.tile_h))
			sdl.y = (cfg.map_h - 1) * cfg.tile_h - sdl.h;
	}
}
