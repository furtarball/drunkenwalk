#include "include/map.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <queue>

Position Position::up(const Map&) const {
	if (y > 0)
		return Position(x, y - 1);
	return *this;
}
Position Position::down(const Map& m) const {
	if (y < static_cast<int>(m.height) - 1)
		return Position(x, y + 1);
	return *this;
}
Position Position::left(const Map&) const {
	if (x > 0)
		return Position(x - 1, y);
	return *this;
}
Position Position::right(const Map& m) const {
	if (x < static_cast<int>(m.width) - 1)
		return Position(x + 1, y);
	return *this;
}
int Position::neighbors(const Map& m, const char f) const {
	int n = 0;
	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
			if (m[x + i][y + j] == f)
				n++;
	return n;
}

void Map::prepareTmp() {
	std::copy(cells.begin(), cells.end() - 1, tmp.begin());
	tmpReady = true;
}
void Map::exportPGM(const std::string& n) const {
	std::ofstream out(n);
	out << "P5 " << width << ' ' << height << " 2 ";
	for (size_t i = 0; i < height; i++)
		for (size_t j = 0; j < width; j++)
			out << cells[j][i];
}
void Map::smoothen(const bool f, const int n) {
	if (!tmpReady)
		prepareTmp();
	for (size_t i = 1; i < height - 1; i++)
		for (size_t j = 1; j < width - 1; j++)
			// swap the cell's state if it has at least n neighbors of the
			// opposite state
			if ((cells[j][i] == f) &&
				(Position(j, i).neighbors(*this, !f) >= n))
				tmp[j][i] = !f;
	apply();
}
void Map::drawWall() {
	if (!tmpReady)
		prepareTmp();
	for (size_t i = 1; i < height - 1; i++)
		for (size_t j = 1; j < width - 1; j++)
			if (!(cells[j][i]) && Position(j, i).neighbors(*this, 1))
				tmp[j][i] = 2;
	apply();
}
void Map::zeroBorder() {
	for (size_t i = 0; i < width; i++) {
		cells[i][0] = 0;
		cells[i][height - 1] = 0;
	}
	for (size_t i = 0; i < height; i++) {
		cells[0][i] = 0;
		cells[width - 1][i] = 0;
	}
}
void Map::floodFill() {
	static std::queue<Position> q;
	q.push(start);
	while (!(q.empty())) {
		Position n = q.front();
		q.pop();
		int x = n.getX();
		int y = n.getY();
		if (cells[x][y] == 1) {
			cells[x][y] = 2;
			q.emplace(x - 1, y);
			q.emplace(x + 1, y);
			q.emplace(x, y - 1);
			q.emplace(x, y + 1);
		}
	}
}
void Map::zero() {
	for (auto&& i : tmp)
		std::fill(i.begin(), i.end(), 0);
	apply();
}
unsigned Map::countWalkable() const {
	unsigned walkable = 0;
	for (auto&& i : cells)
		for (auto&& j : i)
			if (j == 1)
				walkable++;
	return walkable;
}
