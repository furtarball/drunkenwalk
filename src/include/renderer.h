#ifndef RENDERER_H
#define RENDERER_H
#include "map.h"
#include <vector>
#include <memory>
#include "entity.h"
#include "level.h"

const int spriteX = 16, spriteY = 24;
const double scale = 2.0;

class Animation {
  int initialValue;
  unsigned currentFrame;
  unsigned frames;
  int finalValue;
  int delta;
public:
  int currentValue;
  unsigned (Animation::*funcType)();
  unsigned linear() { return initialValue + round((double)delta / frames * currentFrame); }
  unsigned square() { return initialValue + round(delta / pow(frames, 2) * pow(currentFrame, 2)); }
  unsigned log() { return round(delta / ::log(frames + 1) * ::log(currentFrame + 1)) + initialValue; }
  Animation() : initialValue(0), finalValue(0), currentValue(0), funcType(nullptr) {}
  Animation(unsigned (Animation::*f)(), int i, int fin, unsigned l)
    : initialValue(i), currentFrame(0),
      frames(l), finalValue(fin), delta(fin - i), currentValue(0), funcType(f) {}
  unsigned operator()();
  operator bool() { return static_cast<bool>(funcType); }
};

struct FrameRate {
  unsigned fps;
  Uint64 ticks;
  bool skip;
  FrameRate(unsigned f) : fps(f), skip(false) {}
  void updateTicks() { ticks = SDL_GetTicks64(); }
  void measure();
  void set_skip() { // skip one measurement
    skip = true;
  }
  void operator()() {
    updateTicks();
    measure();
  }
};

class Camera {
  int &windowX, &windowY;
public:
  SDL_Rect sdl;
  bool visible(Position p);
  void followPlayer(Position&, Animation&, Animation&);
  Camera(int& x, int& y) : windowX(x), windowY(y) {}
};

class Renderer {
  SDL_Renderer* renderer;
  SDL_Texture* mapLayer, * entityLayer;
  SDL_Texture* environment, * entities;
  std::array<TTF_Font*, 3> fonts;
  void alignment(SDL_Rect&, char, char);
public:
  int windowX, windowY;
  SDL_Window* window;
  Animation fade, mvmtX, mvmtY;
  Camera camera;
  FrameRate fps;
  enum Font { REGULAR16, REGULAR32, BOLD64, FONTS };
  Renderer();
  ~Renderer();
  void renderMapLayer(Map&);
  void drawEntities(EntitiesArray&);
  void print(const std::string&, Font, SDL_Rect&, char, char);
  void drawOSD(std::shared_ptr<Player>, std::array<unsigned, 2>&);
  void prepareAll(Map&, EntitiesArray&, std::shared_ptr<Player>, std::array<unsigned, 2>&);
  void applyFade();
  void present() { SDL_RenderPresent(renderer); }
  void clear() { SDL_RenderClear(renderer); };
private:
  SDL_Texture* textTexture(const std::string&, Font, SDL_Rect&);
};

#endif
