#ifndef MAP_H
#define MAP_H

#include <array>
#include <iostream>

class Map;

class Position {
  int x, y;
public:
  Position(const int p = 0, const int q = 0) : x(p), y(q) {}
  bool operator==(const Position b) const {
    return (x == b.x) && (y == b.y);
  }
  bool operator!=(const Position b) const {
    return !operator==(b);
  }
  int getX() const { return x; }
  int getY() const { return y; }
  Position up();
  Position down();
  Position left();
  Position right();
  int neighbors(const Map&, const char) const;
  friend std::ostream& operator<<(std::ostream& o, const Position& p)
  { return o << '(' << p.getX() << ", " << p.getY() << ')'; }
};

class Map {
public:
  static const int X = 32, Y = 32;
private:
  std::array<std::array<char, Y>, X> cells, tmp;
  const Position start;
  void prepareTmp();
  void apply() { std::copy(tmp.begin(), tmp.end() - 1, cells.begin()); }
public:
  bool tmpReady;
  Map() : start((X / 2), (Y / 2)), tmpReady(false) {}
  Map(const Position& s) : start(s), tmpReady(false) {}
  Position getStart() const { return start; }
  void exportPGM(const std::string&) const;
  void smoothen(const bool, const int);
  void drawWall();
  char operator[](const Position p) const { return cells[p.getX()][p.getY()]; }
  std::array<char, Y>& operator[](const size_t p) { return cells[p]; }
  const std::array<char, Y>& operator[](const size_t p) const { return cells[p]; }
  void zeroBorder();
  void floodFill();
  void zero();
  unsigned countWalkable() const;
};

#endif
