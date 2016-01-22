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
	Tile (const EdgePtrList& tileEdges);
	
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
	EdgeWPtrList edges;
};


#endif