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

const unsigned spriteX = 16, spriteY = 24;
const double scale = 2.0;

struct Animation {
  unsigned (Animation::*funcType)();
  int initialValue;
  unsigned currentFrame;
  int currentValue;
  unsigned frames;
  int finalValue;
  int delta;

  Animation() : funcType(nullptr), initialValue(0), currentValue(0), finalValue(0) {}
  Animation(unsigned (Animation::*f)(), int i, int fin, unsigned l)
    : funcType(f), initialValue(i), currentFrame(0), currentValue(0),
      frames(l), finalValue(fin), delta(fin - i) {}
  unsigned linear() { return initialValue + round((double)delta / frames * currentFrame); }
  unsigned square() { return initialValue + round(delta / pow(frames, 2) * pow(currentFrame, 2)); }
  unsigned log() { return round(delta / ::log(frames + 1) * ::log(currentFrame + 1)) + initialValue; }

  unsigned operator()() {
    if(funcType == nullptr)
      return finalValue;
    currentValue = (this->*(this->funcType))();
    currentFrame++;
    if(currentValue == finalValue)
      funcType = nullptr;
    return currentValue;
  }
  operator bool() { return static_cast<bool>(funcType); }
};

struct FrameRate {
  unsigned fps;
  Uint64 ticks;
  FrameRate(unsigned f) : fps(f) {}
  void updateTicks() { ticks = SDL_GetTicks64(); }
  void measure() {
    static Uint64 prevTicks = ticks;
    static unsigned frames = 0;
    if(ticks - prevTicks >= 1000) {
      fps = frames;
      frames = 0;
      prevTicks = ticks;
    }
    frames++;
  }
  
  void operator()() {
    updateTicks();
    measure();
  }
};

struct Camera {
  SDL_Rect sdl;
  int &windowX, &windowY;
  bool visible(Position p) {
    if((((p.getX() + 1) * spriteX) < static_cast<unsigned>(sdl.x)) ||
       (((p.getY() + 1) * spriteY) < static_cast<unsigned>(sdl.y)) ||
       (((p.getX() - 1) * spriteX) > static_cast<unsigned>(sdl.x + sdl.w)) ||
       (((p.getY() - 1) * spriteY) > static_cast<unsigned>(sdl.y + sdl.h)))
      return false;
    return true;
  }
  void followPlayer(Position& pos, Animation& mvmtX, Animation& mvmtY) {
    sdl = {
      static_cast<int>(pos.getX() * spriteX - (windowX / (scale * 2))),
      static_cast<int>(pos.getY() * spriteY - (windowY / (scale * 2))),
      static_cast<int>(ceil(windowX / scale)),
      static_cast<int>(ceil(windowY / scale))
    };
    sdl.x += mvmtX.currentValue;
    sdl.y += mvmtY.currentValue;
    if(sdl.x < static_cast<int>(spriteX))
      sdl.x = spriteX;
    else if((sdl.x + sdl.w) > static_cast<int>((Map::X - 1) * spriteX)) {
      sdl.x = (Map::X - 1) * spriteX - sdl.w;
    }
    if(sdl.y < static_cast<int>(spriteY))
      sdl.y = spriteY;
    else if((sdl.y + sdl.h) > static_cast<int>((Map::Y - 1) * spriteY))
      sdl.y = (Map::Y - 1) * spriteY - sdl.h;
  }
  Camera(int& x, int& y) : windowX(x), windowY(y) {}
};

struct Renderer {
  int windowX, windowY;
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* mapLayer, * entityLayer;
  SDL_Texture* environment, * entities;
  enum Font { REGULAR16, REGULAR32, BOLD64, FONTS };
  std::array<TTF_Font*, 3> fonts;
  Animation fade, mvmtX, mvmtY;
  Camera camera;
  FrameRate fps;
  Renderer();
  ~Renderer();
  void renderMapLayer(Map&);
  void drawEntities(std::vector<std::shared_ptr<Entity>>);
  SDL_Texture* textTexture(const std::string&, Font, SDL_Rect&);
  void alignment(SDL_Rect&, char, char);
  void print(const std::string&, Font, SDL_Rect&, char, char);
  void drawOSD(std::shared_ptr<Player>, std::array<unsigned, 2>&);
  void prepareAll(Map&, std::vector<std::shared_ptr<Entity>>, std::shared_ptr<Player>, std::array<unsigned, 2>&);
  void applyFade() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, fade());
    SDL_RenderFillRect(renderer, NULL);
  }
  void present() { SDL_RenderPresent(renderer); }
  void clear() { SDL_RenderClear(renderer); };
};

class Game {
public:
  std::unique_ptr<Level> level;
  std::shared_ptr<Player> player;
  Renderer renderer;
  enum Screen { QUIT, LEVEL, DEATH, SCREENS };
  Game() : player(new Player) {}
  void lvl(Screen&);
  void death(Screen&);
  void handleKbdEvents(const Uint8*, SDL_Event&);
  //bool door(std::array<unsigned, 2>&);
  void run();
};

#endif
