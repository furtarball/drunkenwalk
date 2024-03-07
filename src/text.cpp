#include "include/game.h"
#include <array>
#include <sstream>
#include <iomanip>
#include <iostream>
using namespace std;

SDL_Texture* Renderer::textTexture(const string& t, Font f, SDL_Rect& dim) {
  SDL_Surface* ts = TTF_RenderUTF8_Solid_Wrapped(fonts[f], t.c_str(), { 255, 255, 255, 0 }, 0);
  dim.w = ts->w;
  dim.h = ts->h;
  SDL_Texture* tt = SDL_CreateTextureFromSurface(renderer, ts);
  SDL_FreeSurface(ts);
  return tt;
}

// vertical alignment options (the point given in dim will become the...):
// b: bottom, c: center, t: top
// horizontal alignment options (the point will be the...):
// l: left, c: center, r: right
void Renderer::alignment(SDL_Rect& dim, char v, char h) {
  switch(v) {
  case 'b':
    dim.y -= dim.h;
    break;
  case 'c':
    dim.y -= dim.h / 2;
    break;
  default:
    break;
  }
  switch(h) {
  default:
    break;
  case 'c':
    dim.x -= dim.w / 2;
    break;
  case 'r':
    dim.x -= dim.w;
    break;
  }
}

void Renderer::print(const string& t, Font f, SDL_Rect& dst, char vAlign, char hAlign) {
  SDL_Texture* tt = textTexture(t, f, dst);
  alignment(dst, vAlign, hAlign);
  SDL_RenderCopy(renderer, tt, NULL, &dst);
  SDL_DestroyTexture(tt);
}

void Renderer::drawOSD(shared_ptr<Player> player, array<unsigned, 2>& seed) {
  stringstream text;
  text << fps.fps << " fps" << endl;
  text << "Position: (" << player->position.getX() << "; " << player->position.getY() << ')' << endl;
  text << "Map seed: {" << hex << seed[0] << dec << "; " << seed[1] << '}' << endl << endl;
  text << "HP: " << player->hp << '/' << player->maxHp << endl;
  text << "Attack: " << player->attack << endl;
  text << "Defense: " << player->defense << endl << endl;
  text << "Bag:" << endl;
  for(auto&& i : player->bag)
    text << i->name << endl;
  SDL_Rect dst = { 0, 0, 0, 0 };
  print(text.str(), REGULAR16, dst, 't', 'l');
}
