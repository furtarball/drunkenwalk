#ifndef MAP_H
#define MAP_H

#include <iostream>
#include <vector>

class Map;

class Position {
	int x, y;

	public:
	Position(const int p = 0, const int q = 0) : x(p), y(q) {}
	bool operator==(const Position b) const { return (x == b.x) && (y == b.y); }
	bool operator!=(const Position b) const { return !operator==(b); }
	int getX() const { return x; }
	int getY() const { return y; }
	Position up(const Map& m) const;
	Position down(const Map& m) const;
	Position left(const Map& m) const;
	Position right(const Map& m) const;
	int neighbors(const Map&, const char) const;
	friend std::ostream& operator<<(std::ostream& o, const Position& p) {
		return o << '(' << p.getX() << ", " << p.getY() << ')';
	}
};

class Map {
	public:
	size_t width, height;

	private:
	std::vector<std::vector<char>> cells, tmp;
	const Position start;
	void prepareTmp();
	void apply() { std::copy(tmp.begin(), tmp.end() - 1, cells.begin()); }

	public:
	bool tmpReady;
	Map(size_t width, size_t height)
		: width{width}, height{height},
		  cells{static_cast<size_t>(width),
				std::vector<char>(static_cast<size_t>(height))},
		  tmp{cells}, start((width / 2), (height / 2)), tmpReady(false) {}
	Map(size_t width, size_t height, const Position& s)
		: width{width}, height{height}, start(s), tmpReady(false) {}
	Position getStart() const { return start; }
	void exportPGM(const std::string&) const;
	void smoothen(const bool, const int);
	void drawWall();
	char operator[](const Position p) const {
		return cells[p.getX()][p.getY()];
	}
	auto& operator[](const size_t p) { return cells[p]; }
	const auto& operator[](const size_t p) const { return cells[p]; }
	void zeroBorder();
	void floodFill();
	void zero();
	unsigned countWalkable() const;
};

#endif
