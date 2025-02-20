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

Renderer::Renderer()
	: windowX(1024), windowY(768),
	  window{SDL_CreateWindow("Drunken Walk", SDL_WINDOWPOS_UNDEFINED,
									 SDL_WINDOWPOS_UNDEFINED, windowX, windowY,
									 SDL_WINDOW_RESIZABLE)},
	  renderer{SDL_CreateRenderer(
		  window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)},
	  mapLayer{SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
										SDL_TEXTUREACCESS_TARGET,
										Map::X * spriteX, Map::Y * spriteY)},
	  entityLayer{SDL_CreateTexture(
		  renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
		  Map::X * spriteX, Map::Y * spriteY)},
	  environment{IMG_LoadTexture(renderer, "assets/environment.png")},
	  entities{IMG_LoadTexture(renderer, "assets/entities.png")},
	  fonts{TTF_OpenFont("assets/Terminus.ttf", 16),
			TTF_OpenFont("assets/Terminus.ttf", 32),
			TTF_OpenFont("assets/Terminus-Bold.ttf", 64)},
	  fps(60), fade(&Animation::square, 255, 0, fps.fps),
	  camera(windowX, windowY) {

	SDL_SetTextureBlendMode(entityLayer, SDL_BLENDMODE_BLEND);
	// PRERR(fonts[0] = TTF_OpenFont("assets/gallant12x22.ttf", 22));
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void Renderer::renderMapLayer(Map& map) {
	SDL_SetRenderTarget(renderer, mapLayer);
	SDL_Rect src = {0, 0, spriteX, spriteY};
	SDL_Rect dst = src;
	for (size_t i = 0; i < Map::Y; i++) {
		for (size_t j = 0; j < Map::X; j++) {
			src.x = map[j][i] * spriteX;
			SDL_RenderCopy(renderer, environment, &src, &dst);
			dst.x += spriteX;
		}
		dst.x = 0;
		dst.y += spriteY;
	}
	SDL_SetRenderTarget(renderer, NULL);
}

void Renderer::drawEntities(EntitiesArray& earr) {
	static Uint64 prevTicks = fps.ticks;
	static int frame = 0;
	if (fps.ticks >= prevTicks + 1000) {
		frame++;
		prevTicks = fps.ticks;
	}

	SDL_SetRenderTarget(renderer, entityLayer);
	SDL_RenderClear(renderer);
	for (auto i = earr.end() - 1; i >= earr.begin(); i--) {
		int x = (*i)->position.getX(), y = (*i)->position.getY();
		if (camera.visible((*i)->position)) {
			SDL_Rect offset = {(*i)->offset * spriteX,
							   (frame % (*i)->frames) * spriteY, spriteX,
							   spriteY};
			SDL_Rect pos = {x * spriteX, y * spriteY, spriteX, spriteY};
			if (i == earr.player()) {
				pos.x += mvmtX();
				pos.y += mvmtY();
			}
			SDL_RenderCopy(renderer, entities, &offset, &pos);
		}
	}
	SDL_SetRenderTarget(renderer, NULL);
}

void Renderer::prepareAll(Map& map, EntitiesArray& earr,
						  std::shared_ptr<Player> player, std::array<unsigned, 2>& seed) {
	renderMapLayer(map);
	SDL_Rect targetRect;
	targetRect.w = static_cast<int>(ceil(camera.sdl.w * scale));
	targetRect.h = static_cast<int>(ceil(camera.sdl.h * scale));
	targetRect.x = static_cast<int>(ceil((windowX - targetRect.w) / 2.0));
	targetRect.y = static_cast<int>(ceil((windowY - targetRect.h) / 2.0));
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
	if ((((p.getX() + 1) * spriteX) < sdl.x) ||
		(((p.getY() + 1) * spriteY) < sdl.y) ||
		(((p.getX() - 1) * spriteX) > (sdl.x + sdl.w)) ||
		(((p.getY() - 1) * spriteY) > (sdl.y + sdl.h)))
		return false;
	return true;
}

void Camera::followPlayer(Position& pos, Animation& mvmtX, Animation& mvmtY) {
	sdl = {static_cast<int>(pos.getX() * spriteX - (windowX / (scale * 2))),
		   static_cast<int>(pos.getY() * spriteY - (windowY / (scale * 2))),
		   static_cast<int>(ceil(windowX / scale)),
		   static_cast<int>(ceil(windowY / scale))};
	sdl.x += mvmtX.currentValue;
	sdl.y += mvmtY.currentValue;
	// check if the map is narrower (shorter) than the viewport
	// if not, check if the above has tried to move it outside the map
	if (((Map::X - 1) * spriteX) < sdl.w) {
		sdl.w = ((Map::X - 1) * spriteX);
		sdl.x = 0;
	} else {
		if (sdl.x < spriteX)
			sdl.x = spriteX;
		else if ((sdl.x + sdl.w) > ((Map::X - 1) * spriteX))
			sdl.x = (Map::X - 1) * spriteX - sdl.w;
	}
	if (((Map::Y - 1) * spriteY) < sdl.h) {
		sdl.h = ((Map::Y - 1) * spriteY);
		sdl.y = 0;
	} else {
		if (sdl.y < spriteY)
			sdl.y = spriteY;
		else if ((sdl.y + sdl.h) > ((Map::Y - 1) * spriteY))
			sdl.y = (Map::Y - 1) * spriteY - sdl.h;
	}
}
