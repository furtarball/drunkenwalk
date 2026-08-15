#include "include/game.h"
#include <array>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

void Renderer::alignment(SDL_Rect& dim, AlignV v, AlignH h) {
	switch (v) {
	case BOTTOM:
		dim.y -= dim.h;
		break;
	case CENTERV:
		dim.y -= dim.h / 2;
		break;
	default:
		break;
	}
	switch (h) {
	default:
		break;
	case CENTERH:
		dim.x -= dim.w / 2;
		break;
	case RIGHT:
		dim.x -= dim.w;
		break;
	}
}

Wrapped<SDL_Texture> Renderer::textTexture(
	const std::string& t, Font f, SDL_Rect& dim) {
	Wrapped<SDL_Surface> ts{TTF_RenderUTF8_Solid_Wrapped(
		fonts[f], t.c_str(), {255, 255, 255, 0}, 0)};
	dim.w = ts->w;
	dim.h = ts->h;
	return SDL_CreateTextureFromSurface(renderer, ts);
}

void Renderer::print(
	const std::string& t, Font f, SDL_Rect& dst, AlignV v, AlignH h) {
	Wrapped<SDL_Texture> tt = textTexture(t, f, dst);
	alignment(dst, v, h);
	SDL_RenderCopy(renderer, tt, NULL, &dst);
}

void Renderer::drawOSD(std::shared_ptr<Player> player, Seed& seed) {
	std::stringstream text;
	text << fps.fps << " fps\n";
	text << "Position: (" << player->position.getX() << "; "
		 << player->position.getY() << ")\n";
	text << "Map seed: {" << std::hex << seed[0] << std::dec << "; " << seed[1]
		 << "}\n\n";
	text << "HP: " << player->hp << '/' << player->maxhp << std::endl;
	SDL_Rect dst = {0, 0, 0, 0};
	print(text.str(), REGULAR16, dst, TOP, LEFT);
}
