#include <fstream>
#include <algorithm>
#include <queue>
#include "include/map.h"
#include <iostream>

Position Position::up() {
  if(y > 0)
    return Position(x, y - 1);
  return *this;
}
Position Position::down() {
  if(y < Map::Y - 1)
    return Position(x, y + 1);
  return *this;
}
Position Position::left() {
  if(x > 0)
    return Position(x - 1, y);
  return *this;
}
Position Position::right() {
  if(x < Map::X - 1)
    return Position(x + 1, y);
  return *this;
}
int Position::neighbors(const Map& m, const char f) const {
  int n = 0;
  for(int i = -1; i <= 1; i++)
    for(int j = -1; j <= 1; j++)
      if(m[x + i][y + j] == f)
	n++;
  return n;
}

void Map::prepareTmp() {
  std::copy(cells.begin(), cells.end() - 1, tmp.begin());
  tmpReady = true;
}
void Map::exportPGM(const std::string& n) const {
  std::ofstream out(n);
  out << "P5 " << X << ' ' << Y << " 2 ";
  for(size_t i = 0; i < Y; i++)
    for(size_t j = 0; j < X; j++)
      out << cells[j][i];
}
void Map::smoothen(const bool f, const int n) {
  if(!tmpReady) prepareTmp();
  for(size_t i = 1; i < Y - 1; i++)
    for(size_t j = 1; j < X - 1; j++)
      // swap the cell's state if it has at least n neighbors of the opposite state
      if((cells[j][i] == f) && (Position(j, i).neighbors(*this, !f) >= n))
	tmp[j][i] = !f;
  apply();
}
void Map::drawWall() {
  if(!tmpReady) prepareTmp();
  for(size_t i = 1; i < Y - 1; i++)
    for(size_t j = 1; j < X - 1; j++)
      if(!(cells[j][i]) && Position(j, i).neighbors(*this, 1))
	tmp[j][i] = 2;
  apply();
}
void Map::zeroBorder() {
  for(size_t i = 0; i < X; i++) {
    cells[i][0] = 0;
    cells[i][Y - 1] = 0;
  }
  for(size_t i = 0; i < Y; i++) {
    cells[0][i] = 0;
    cells[X - 1][i] = 0;
  }
}
void Map::floodFill() {
  static std::queue<Position> q;
  q.push(start);
  while(!(q.empty())) {
    Position n = q.front();
    q.pop();
    int x = n.getX();
    int y = n.getY();
    if(cells[x][y] == 1) {
      cells[x][y] = 2;
      q.emplace(x - 1, y);
      q.emplace(x + 1, y);
      q.emplace(x, y - 1);
      q.emplace(x, y + 1);
    }
  }
}
void Map::zero() {
  for(auto&& i : tmp)
    i.fill(0);
  apply();
}
unsigned Map::countWalkable() const {
  unsigned walkable = 0;
  for(auto&& i : cells)
    for(auto&& j : i)
      if(j == 1)
	walkable++;
  return walkable;
}
