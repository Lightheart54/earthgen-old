#pragma warning(disable:4996)
#include "corner.h"
#include "tile.h"
#include "edge.h"

Corner::Corner(const PosVector & _position):position(_position)
{
}

EdgePtrList Corner::getEdges() const
{
	return lockList(edges);
}

TilePtrList Corner::getTiles() const
{
	TilePtrList neighbors;
	for (EdgeWPtr edge : edges)
	{
		TilePtrList edgeTiles = edge.lock()->getTiles();
		neighbors.insert(neighbors.end(), edgeTiles.begin(), edgeTiles.end());
	}
	auto newEndIt = std::unique(neighbors.begin(), neighbors.end());
	neighbors.erase(newEndIt, neighbors.end());
	return neighbors;
}

CornerPtrList Corner::getCorners() const
{
	CornerPtrList connected;
	for (const EdgeWPtr edge : edges)
	{
		EdgePtr edgeptr = edge.lock();
		CornerPtrList edgeCorners = edge.lock()->getEndPoints();
		if (edgeCorners.front().get() == this)
		{
			connected.push_back(edgeCorners.back());
		}
		else
		{
			connected.push_back(edgeCorners.front());
		}
	}
	return connected;
}

PosVector Corner::getPosition() const
{
	return position;
}

CornerPtrList Corner::getConnectingCorners(const CornerPtr& targetCorner) const
{
	CornerPtrList myCorners = getCorners();
	CornerPtrList otherCorners = targetCorner->getCorners();
	std::sort(myCorners.begin(), myCorners.end());
	std::sort(otherCorners.begin(), otherCorners.end());
	CornerPtrList resultCorners;
	std::set_intersection(myCorners.begin(), myCorners.end(), otherCorners.begin(), otherCorners.end(), std::back_inserter(resultCorners));
	return resultCorners;
}

EdgePtr Corner::getConnectingEdge(const CornerPtr& targetCorner) const
{
	EdgePtrList myEdges = getEdges();
	EdgePtrList otherEdges = targetCorner->getEdges();
	std::sort(myEdges.begin(), myEdges.end());
	std::sort(otherEdges.begin(), otherEdges.end());
	EdgePtrList resultEdges;
	std::set_intersection(myEdges.begin(), myEdges.end(), otherEdges.begin(), otherEdges.end(), std::back_inserter(resultEdges));
	if (!resultEdges.empty())
	{
		return resultEdges.front();
	}
	return nullptr;
}
