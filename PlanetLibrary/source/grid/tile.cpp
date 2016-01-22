#pragma warning(disable:4996)
#include "tile.h"
#include "corner.h"
#include "edge.h"
#include "StateBase.h"

Tile::Tile(const EdgePtrList& tileEdges)
{
	std::transform(tileEdges.begin(), tileEdges.end(), std::back_inserter(edges),
		[](const EdgePtr& cPtr)->EdgeWPtr { return EdgeWPtr(cPtr); });
	std::vector<PosVector> edgePos;
	std::transform(tileEdges.begin(), tileEdges.end(), std::back_inserter(edgePos),
		[](const EdgePtr& cPtr)->PosVector { return cPtr->getPosition(); });
	position = getAveragedVector(edgePos);
}

Tile::~Tile()
{
}

TilePtrList Tile::getNeighbors() const
{
	TilePtrList neighbors;
	for (const EdgeWPtr& edge : edges)
	{
		TilePtrList edgeTiles = edge.lock()->getTiles();
		if (edgeTiles.front().get() == this)
		{
			neighbors.push_back(edgeTiles.back());
		}
		else
		{
			neighbors.push_back(edgeTiles.front());
		}
	}

	return neighbors;
}

EdgePtrList Tile::getEdges() const
{
	return lockList(edges);
}

CornerPtrList Tile::getCorners() const
{
	CornerPtrList corners;
	for (const EdgeWPtr& edge : edges)
	{
		CornerPtrList edgeCorners = edge.lock()->getEndPoints();
		corners.insert(corners.end(), edgeCorners.begin(), edgeCorners.end());
	}
	std::sort(corners.begin(), corners.end());
	std::unique(corners.begin(), corners.end());
	return corners;
}

PosVector Tile::getPosition() const
{
	return position;
}

TileState * Tile::getState(const std::string & stateName) const
{
	if (tileStates.find(stateName)==tileStates.end())
	{
		return nullptr;
	}
	else
	{
		return tileStates.at(stateName);
	}
}

bool Tile::addState(TileState * tileState)
{
	if (tileStates.find(tileState->getStateName()) == tileStates.end())
	{
		tileStates.insert(std::make_pair(tileState->getStateName(), tileState));
		return true;
	}
	return false;
}
