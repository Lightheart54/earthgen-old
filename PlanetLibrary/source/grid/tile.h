#ifndef tile_h
#define tile_h

#include <vector>
#include <unordered_map>
#include "PositionVector.h"
#include "StateBase.h"

class Tile {
public:
	//!Base Constructor
	//! \param[in] create a tile with a given position vector
	//! \param[in] cornerList the list of corners to make the tile from
	Tile (const PosVector& tilePosition, const EdgePtrList& tileEdges);

	//!Subdivision Constructor creates a tile from an existing corner point
	//! \param[in] baseCorner the corner point to create the subdivision side from
	static TilePtr createOnSubdividedGrid(const CornerPtr& baseCorner);

	~Tile();

	TilePtrList getNeighbors() const;
	EdgePtrList getEdges() const;
	CornerPtrList getCorners() const;
	PosVector getPosition() const;
	TileState* getState(const std::string& stateName) const;
	bool addState(TileState* tileState);

protected:
	friend class Grid;

	std::unordered_map<std::string,TileState*> tileStates;
	
	PosVector position;
	CornerWPtrList corners;
	EdgeWPtrList edges;
};


#endif