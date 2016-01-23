#pragma warning(disable:4996)
#include "grid/tile.h"
#include "grid/corner.h"
#include "grid/edge.h"
#include "grid/StateBase.h"

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

double Tile::getArea() const
{
	EdgePtrList myEdges = getEdges();
	double area = 0.0;
	for (const EdgePtr& edge : myEdges)
	{
		PosVector vec1 = edge->getEndPoints().front()->getPosition() - position;
		PosVector vec2 = edge->getEndPoints().back()->getPosition() - position;
		PosVector v1xv2 = cross_product(vec1, vec2);
		area += 0.5 * sqrt(boost::numeric::ublas::inner_prod(v1xv2, v1xv2));
	}
	return area;
}

double Tile::getEnclosedVolume() const
{
	EdgePtrList myEdges = getEdges();
	double volume = 0.0;
	for (const EdgePtr& edge : myEdges)
	{
		PosVector vec1 = edge->getEndPoints().front()->getPosition();
		PosVector vec2 = edge->getEndPoints().back()->getPosition();
		PosVector v1xv2 = cross_product(vec1, vec2);
		volume += std::abs(boost::numeric::ublas::inner_prod(v1xv2, position));
	}
	return volume;
}
