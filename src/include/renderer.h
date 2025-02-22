#ifndef RENDERER_H
#define RENDERER_H
#include "entity.h"
#include "level.h"
#include "map.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <memory>
#include <vector>

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
	unsigned linear() {
		return initialValue + round((double)delta / frames * currentFrame);
	}
	unsigned square() {
		return initialValue +
			   round(delta / pow(frames, 2) * pow(currentFrame, 2));
	}
	unsigned log() {
		return round(delta / ::log(frames + 1) * ::log(currentFrame + 1)) +
			   initialValue;
	}
	Animation()
		: initialValue(0), finalValue(0), currentValue(0), funcType(nullptr) {}
	Animation(unsigned (Animation::*f)(), int i, int fin, unsigned l)
		: initialValue(i), currentFrame(0), frames(l), finalValue(fin),
		  delta(fin - i), currentValue(0), funcType(f) {}
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

// Macro for instantiating std::unique_ptr with default deleters
template <typename T>
class Wrapped : public std::unique_ptr<T, void (*)(T*)> {};
#define WRAPPED(type, deleter)                                                 \
	template <>                                                                \
	class Wrapped<type> : public std::unique_ptr<type, void (*)(type*)> {      \
		public:                                                                \
		Wrapped(type* p)                                                       \
			: std::unique_ptr<type, void (*)(type*)>{p, deleter} {             \
			if (p == nullptr)                                                  \
				throw std::runtime_error{SDL_GetError()};                      \
		}                                                                      \
		operator type*() { return get(); }                                     \
	};
WRAPPED(SDL_Window, SDL_DestroyWindow);
WRAPPED(SDL_Renderer, SDL_DestroyRenderer);
WRAPPED(SDL_Surface, SDL_FreeSurface);
WRAPPED(SDL_Texture, SDL_DestroyTexture);
WRAPPED(TTF_Font, TTF_CloseFont);

class Renderer {
	// A drawback of wrapping SDL object pointers in STL smart pointers is that
	// SDL_Init() and SDL_Quit() now have to be called from outside Renderer's
	// constructor/destructor
	class SDLGuard {
		public:
		SDLGuard();
		~SDLGuard();
	};
	SDLGuard sdlguard;

	public:
	enum Font { REGULAR16, REGULAR32, BOLD64, FONTS };
	int windowX, windowY;
	Wrapped<SDL_Window> window;

	private:
	Wrapped<SDL_Renderer> renderer;
	Wrapped<SDL_Texture> mapLayer, entityLayer, environment, entities;
	std::array<Wrapped<TTF_Font>, 3> fonts;
	Wrapped<SDL_Texture> textTexture(const std::string& t, Font f,
									 SDL_Rect& dim);

	public:
	FrameRate fps;
	Animation fade, mvmtX, mvmtY;
	Camera camera;
	Renderer();
	void renderMapLayer(Map&);
	void drawEntities(EntitiesArray&);
	void print(const std::string&, Font, SDL_Rect&, char, char);
	void drawOSD(std::shared_ptr<Player>, std::array<unsigned, 2>&);
	void prepareAll(Map&, EntitiesArray&, std::shared_ptr<Player>,
					std::array<unsigned, 2>&);
	void applyFade();
	void present() { SDL_RenderPresent(renderer); }
	void clear() { SDL_RenderClear(renderer); };
};

#endif
