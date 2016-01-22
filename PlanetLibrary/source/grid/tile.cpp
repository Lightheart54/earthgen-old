#pragma warning(disable:4996)
#include "tile.h"
#include "corner.h"
#include "edge.h"
#include "StateBase.h"

Tile::Tile(const PosVector& tilePosition, const EdgePtrList& tileEdges)
	: position(tilePosition)
{
	std::transform(tileEdges.begin(), tileEdges.end(), std::back_inserter(edges),
		[](const EdgePtr& cPtr)->EdgeWPtr { return EdgeWPtr(cPtr); });
}

TilePtr Tile::createOnSubdividedGrid(const CornerPtr& baseCorner)
{
	PosVector myPos = baseCorner->getPosition();
	EdgePtrList cornerEdges = baseCorner->getEdges();
	CornerPtrList myCorners;
	EdgePtrList myEdges;
	for (const EdgePtr& edge : cornerEdges)
	{
		CornerPtrList edgeCorners = edge->getEndPoints();
		if (edgeCorners.front() == baseCorner)
		{
			myCorners.push_back(edgeCorners.back());
		}
		else
		{
			myCorners.push_back(edgeCorners.front());
		}
		myCorners.push_back(nullptr);
	}
	
	for (size_t i = 0; i < 6; i++)
	{
		if ((i+1) % 2 == 1)
		{
			size_t next_corner = i < 5 ? i : 0;
			myCorners[i + 1] = myCorners[i]->getConnectingCorners(myCorners[next_corner]).front();
		}
		size_t next_corner = i == 5 ? 0 : i + 1;
		myEdges.push_back(myCorners[i]->getConnectingEdge(myCorners[next_corner]));
	}

	//test to see if the resulting edge loop is in the same direction as the position vector
	PosVector vec1 = myCorners[1]->getPosition() - myCorners[0]->getPosition();
	PosVector vec2 = myCorners[2]->getPosition() - myCorners[1]->getPosition();
	PosVector xProd = cross_product(vec1, vec2);
	double dotProd = boost::numeric::ublas::inner_prod(xProd, myPos);
	if (dotProd < 0)
	{
		// it its not reverse the vectors
		std::reverse(myCorners.begin(), myCorners.end());
		std::reverse(myEdges.begin(), myEdges.end());
	}
	return std::make_shared<Tile>(myPos, myEdges);
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
