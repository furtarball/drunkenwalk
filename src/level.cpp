#include "include/level.h"
#include "include/entity.h"
#include "include/mapgen.h"
#include "include/config.h"

void EntitiesArray::insert(std::shared_ptr<Door> d) {
	data.insert(door_end(), d);
	doors++;
}
void EntitiesArray::insert(std::shared_ptr<Item> i) {
	data.insert(item_end(), i);
	items++;
}
void EntitiesArray::insert(std::shared_ptr<Player> p) {
	if ((data.size() > 0) && (typeid(*player()) == typeid(Player)))
		*player() = p;
	else
		data.insert(player(), p);
	players = 1;
}
void EntitiesArray::insert(std::shared_ptr<Enemy> e) {
	data.insert(mob_end(), e);
	mobs++;
}
decltype(EntitiesArray::data)::iterator
EntitiesArray::erase(decltype(EntitiesArray::data)::iterator pos) {
	size_t n = pos - begin();
	if ((pos >= door0()) && (pos < door_end()))
		doors--;
	else if ((pos >= item0()) && (pos < item_end()))
		items--;
	else if (pos == player())
		players = 0;
	else if ((pos >= mob0()) && (pos < mob_end()))
		mobs--;
	data.erase(pos);
	return begin() + n;
}

Level::Level(size_t map_w, size_t map_h, Seed& a, std::shared_ptr<Player>& p,
			 const Config& cfg)
	: map{map_w, map_h}, seed(a.begin(), a.end()), player(p) {
	switch (static_cast<MapType>(a[0] % static_cast<unsigned>(MAPTYPES))) {
	case CAVE_REGULAR:
		gen = std::make_unique<DrunkenWalk>(seed, map, entities);
		break;
	case CAVE_CORRIDOR:
		gen = std::make_unique<TargetedDrunkenWalk>(seed, map, entities);
		break;
	default:
		throw std::runtime_error{"Unable to create a map generator."};
	}
	entities.insert(player);
	gen->generateMap();
	std::ifstream items_f{cfg.asset_path(cfg.item_types)};
	nlohmann::json itemTypes = nlohmann::json::parse(items_f);
	std::ifstream enemies_f{cfg.asset_path(cfg.enemy_types)};
	nlohmann::json enemyTypes = nlohmann::json::parse(enemies_f);
	gen->populate(itemTypes, enemyTypes);
}
bool Level::collision(const Position p) {
	if (map[p] != 1)
		return true;
	for (auto&& i : entities)
		if ((i->position == p) && i->collision)
			return true;
	return false;
}

bool Level::grab() {
	for (auto i = entities.item0(); i < entities.item_end(); i++) {
		std::shared_ptr<Item> itemPtr = std::dynamic_pointer_cast<Item>(*i);
		if (itemPtr->position == player->position) {
			player->bag.push_back(itemPtr);
			entities.erase(i);
			return true;
		}
	}
	return false;
}

void Level::fight() {
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			Position p(player->position.getX() + j,
					   player->position.getY() + i);
			for (auto k = entities.mob0(); k < entities.mob_end(); k++) {
				if ((*k)->position == p) {
					std::shared_ptr<Enemy> enemy =
						std::dynamic_pointer_cast<Enemy>(*k);
					unsigned damage =
						std::lround((player->attack * 100.0) /
									static_cast<double>(enemy->defense + 100));
					if (enemy->hp > damage)
						enemy->hp -= damage;
					else
						enemy->hp = 0;
					if (enemy->hp == 0) {
						entities.erase(k);
					} else {
						damage = std::lround(
							(enemy->attack * 100.0) /
							static_cast<double>(player->defense + 100));
						if (player->hp > damage)
							player->hp -= damage;
						else
							player->hp = 0;
					}
				}
			}
		}
	}
}
