#ifndef CONFIG_H
#define CONFIG_H
#include "entity.h"
#include <nlohmann/json.hpp>
#include <string>

struct Config {
	int map_w{32};
	int map_h{32};
	int window_w{1024};
	int window_h{768};
	std::string assets_dir{"assets"};
	std::string environment{"environment.png"};
	std::string entities{"entities.png"};
	int tile_w{16};
	int tile_h{24};
	double scale{2.0};
	std::string font_regular_file{"Terminus.ttf"};
	std::string font_medium_file{"Terminus.ttf"};
	std::string font_big_file{"Terminus-Bold.ttf"};
	int font_regular_size{16};
	int font_medium_size{32};
	int font_big_size{64};
	std::string player_spritesheet{"player.png"};
	Sprite player_sprite_object{16, 16, 25, 1, 150};
	Config() = default;
	Config(const char* json_path);
	std::string asset_path(std::string file) const {
		return assets_dir + '/' + file;
	}
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
	Config, window_w, window_h, assets_dir, environment, entities, tile_w,
	tile_h, scale, font_regular_file, font_medium_file, font_big_file,
	font_regular_size, font_medium_size, font_big_size, player_spritesheet,
	player_sprite_object);

#endif
