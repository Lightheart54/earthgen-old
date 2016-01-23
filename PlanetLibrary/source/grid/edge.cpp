#pragma warning(disable:4996)
#include "grid/edge.h"
#include "grid/tile.h"
#include "grid/corner.h"
#include <initializer_list>

Edge::Edge(CornerPtr startPoint, CornerPtr endPoint)
{
	corners = { startPoint,endPoint};
	PosVector spPos = startPoint->getPosition();
	PosVector epPos = endPoint->getPosition();
	double mag = std::sqrt(boost::numeric::ublas::inner_prod(spPos, spPos));
	position = getAveragedVector({ spPos,epPos });
}

TilePtrList Edge::getTiles() const
{
	return lockList(tiles);
}

CornerPtrList Edge::getEndPoints() const
{
	return lockList(corners);
}

PosVector Edge::getPosition() const
{
	return position;
}

EdgeState * Edge::getState(const std::string & stateName) const
{
	if (edgeStates.find(stateName) == edgeStates.end())
	{
		return nullptr;
	}
	else
	{
		return edgeStates.at(stateName);
	}
}

bool Edge::addState(EdgeState * edgeState)
{
	if (edgeStates.find(edgeState->getStateName()) == edgeStates.end())
	{
		edgeStates.insert(std::make_pair(edgeState->getStateName(),edgeState));
		return true;
	}
	return false;
}

double Edge::getLength() const
{
	PosVector lVec = corners.front().lock()->getPosition() - corners.back().lock()->getPosition();
	return std::sqrt(boost::numeric::ublas::inner_prod(lVec, lVec));
}
