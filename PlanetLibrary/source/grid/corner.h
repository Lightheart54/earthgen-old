#ifndef corner_h
#define corner_h

#include <vector>
#include "PositionVector.h"

class Corner {
public:
	Corner (const PosVector& _position);
	
	EdgePtrList getEdges() const;
	TilePtrList getTiles() const;
	CornerPtrList getCorners() const;
	PosVector getPosition() const;
	CornerPtrList getConnectingCorners(const CornerPtr& targetCorner) const;
	EdgePtr getConnectingEdge(const CornerPtr& targetCorner) const;

protected:
	friend class Grid;

	PosVector position;
	EdgeWPtrList edges;
};

#endif