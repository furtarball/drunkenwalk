#include "include/entity.h"
#include <memory>
#include <nlohmann/json.hpp>

unsigned Door::count = 0;

void Sprite::progress_frames(const FrameRate& fps) {
	if (frames <= 1)
		return;
	if (fps.ticks < next)
		return;
	auto advance{(fps.ticks - next) / frame_ms};
	curr_frame = (curr_frame + advance + 1) % frames;
	next = fps.ticks + frame_ms;
}

Enemy::Enemy(Position pos, const nlohmann::json& j)
	: Entity{pos, true, {j.at("Sprite").template get<Sprite>()}} {
	j.at("Name").get_to(name);
	j.at("MaxHP").get_to(maxhp);
	j.at("Attack").get_to(attack);
	j.at("Defense").get_to(defense);
	hp = maxhp;
}
Item::Item(Position pos, const nlohmann::json& j) : Entity{pos, false, {80}} {
	j.at("Name").get_to(name);
	j.at("HP").get_to(hp);
	j.at("MaxHP").get_to(maxhp);
	j.at("Attack").get_to(attack);
	j.at("Defense").get_to(defense);
	if (j.find("Type") != j.end())
		j.at("Type").get_to(type);
};

void Player::use(std::shared_ptr<Item> i) {
	add_stats(*i);
	if (i->type == Item::WEAPON) {
		if (weapon)
			subtract_stats(*weapon);
		weapon = i;
	} else if (i->type == Item::ARMOR) {
		if (armor)
			subtract_stats(*armor);
		armor = i;
	}
}

void Player::add_stats(Item& i) {
	add_maxHp(i.maxhp);
	add_hp(i.hp);
	add_attack(i.attack);
	add_defense(i.defense);
}

void Player::subtract_stats(Item& i) {
	add_maxHp(-(i.maxhp));
	add_attack(-(i.attack));
	add_defense(-(i.defense));
}

void Player::add_hp(int h) {
	hp += h;
	if (hp < 0)
		hp = 0;
	if (hp > maxhp)
		hp = maxhp;
}

void Player::add_maxHp(int m) { maxhp += m; }

void Player::add_attack(int a) { attack += a; }

void Player::add_defense(int d) { defense += d; }
