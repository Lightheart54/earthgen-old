#pragma once
#include <string>

class Edge;
class Tile;

template<class _StateItem>
class StateBase
{
public:
	StateBase(const std::string& _stateName, _StateItem* _item)
	{
		stateName = _stateName;
		item = _item;
	}
	virtual ~StateBase() = 0;

	std::string getStateName() const
	{
		return stateName;
	}

	_StateItem* getItem() const
	{
		return item;
	}
protected:
	std::string stateName;
	_StateItem* item;
};

typedef StateBase<Edge> EdgeState;
typedef StateBase<Tile> TileState;
