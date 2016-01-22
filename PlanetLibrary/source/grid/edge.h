#ifndef edge_h
#define edge_h

#include <vector>
#include <unordered_map>
#include "PositionVector.h"
#include "StateBase.h"

class Edge {
	public:
		Edge (CornerPtr startPoint, CornerPtr endPoint);

		TilePtrList getTiles() const;
		CornerPtrList getEndPoints() const;
		PosVector getPosition() const;

		EdgeState* getState(const std::string& stateName) const;
		bool addState(EdgeState* edgeState);

	protected:
		friend class Grid;
	
		PosVector position;
		TileWPtrList tiles;
		CornerWPtrList corners;
		std::unordered_map<std::string, EdgeState*> edgeStates;
};

#endif
