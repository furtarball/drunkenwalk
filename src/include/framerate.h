#ifndef FRAMERATE_H
#define FRAMERATE_H
#include <SDL2/SDL_timer.h>

struct FrameRate {
	unsigned fps;
	Uint64 ticks;
	bool skip;
	explicit FrameRate(unsigned f) : fps(f), skip(false) {}
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

#endif
