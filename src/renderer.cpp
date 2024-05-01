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

#define PRERR(instruction) if((instruction) == nullptr) cerr << "Error: " << #instruction << endl;

Renderer::Renderer() : windowX(640), windowY(400), fade(&Animation::square, 255, 0, 60), camera(windowX, windowY), fps(60) {
  if(SDL_Init(SDL_INIT_VIDEO) != 0)
    cerr << "SDL_Init error" << endl;
  //PRERR(window = SDL_CreateWindow("Drunken Walk", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowX * spriteX * scale, windowY * spriteY * scale, 0));
  PRERR(window = SDL_CreateWindow("Drunken Walk", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowX, windowY, SDL_WINDOW_RESIZABLE));
  PRERR(renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
  PRERR(mapLayer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, Map::X * spriteX, Map::Y * spriteY));
  PRERR(entityLayer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, Map::X * spriteX, Map::Y * spriteY));
  SDL_SetTextureBlendMode(entityLayer, SDL_BLENDMODE_BLEND);
  if(IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    cerr << "IMG_Init error" << endl;
  PRERR(environment = IMG_LoadTexture(renderer, "assets/environment.png"));
  PRERR(entities = IMG_LoadTexture(renderer, "assets/entities.png"));
  IMG_Quit();
  TTF_Init();
  PRERR(fonts[0] = TTF_OpenFont("assets/Terminus.ttf", 16));
  //PRERR(fonts[0] = TTF_OpenFont("assets/gallant12x22.ttf", 22));
  PRERR(fonts[1] = TTF_OpenFont("assets/Terminus.ttf", 32));
  PRERR(fonts[2] = TTF_OpenFont("assets/Terminus-Bold.ttf", 64));
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
}

void Renderer::renderMapLayer(Map& map) {
  SDL_SetRenderTarget(renderer, mapLayer);
  SDL_Rect src = { 0, 0, spriteX, spriteY };
  SDL_Rect dst = src;
  for(size_t i = 0; i < Map::Y; i++) {
    for(size_t j = 0; j < Map::X; j++) {
      src.x = map[j][i] * spriteX;
      SDL_RenderCopy(renderer, environment, &src, &dst);
      dst.x += spriteX;
    }
    dst.x = 0;
    dst.y += spriteY;
  }
  SDL_SetRenderTarget(renderer, NULL);
}

void Renderer::drawEntities(vector<shared_ptr<Entity>> evctr) {
  static Uint64 prevTicks = fps.ticks;
  static unsigned frame = 0;
  if(fps.ticks >= prevTicks + 1000) {
    frame++;
    prevTicks = fps.ticks;
  }

  SDL_SetRenderTarget(renderer, entityLayer);
  SDL_RenderClear(renderer);
  for(int i = evctr.size() - 1; i >= 0; i--) {
    unsigned x = evctr[i]->position.getX(), y = evctr[i]->position.getY();
    if(camera.visible(evctr[i]->position)) {
      SDL_Rect offset = { static_cast<int>(evctr[i]->offset * spriteX), static_cast<int>((frame % evctr[i]->frames) * spriteY), spriteX, spriteY };
      SDL_Rect pos = {
	static_cast<int>(x * spriteX),
	static_cast<int>(y * spriteY),
	spriteX,
	spriteY
      };
      if(i == 0) {
	pos.x += mvmtX();
	pos.y += mvmtY();
      }
      SDL_RenderCopy(renderer, entities, &offset, &pos);
    }
  }
  SDL_SetRenderTarget(renderer, NULL);
}

void Renderer::prepareAll(Map& map, vector<shared_ptr<Entity>> entitiesVctr, shared_ptr<Player> player, array<unsigned, 2>& seed) {
  renderMapLayer(map);
  SDL_Rect targetRect = {
    0,
    0,
    camera.sdl.w * static_cast<int>(scale),
    camera.sdl.h * static_cast<int>(scale)
  };
  SDL_RenderCopy(renderer, mapLayer, &camera.sdl, &targetRect);
  drawEntities(entitiesVctr);
  camera.followPlayer(player->position, mvmtX, mvmtY);
  SDL_RenderCopy(renderer, entityLayer, &camera.sdl, &targetRect);
  drawOSD(player, seed);
}

Renderer::~Renderer() {
  for(auto&& i : fonts)
    TTF_CloseFont(i);
  TTF_Quit();
  SDL_DestroyTexture(environment);
  SDL_DestroyTexture(entities);
  SDL_DestroyTexture(mapLayer);
  SDL_DestroyTexture(entityLayer);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
