#ifndef grid_h
#define grid_h

#include <map>
#include "tile.h"
#include "corner.h"
#include "edge.h"

class Grid
{
	static Grid createGridWithEdgeLength(const double& _radius, const double& desiredTileArea);
	Grid(const double& _radius, const unsigned int& numSubdivisions);
	~Grid();

	TilePtr getClosestTile(const PosVector& vec) const;
	EdgePtr getClosestEdge(const PosVector& vec) const;
	CornerPtr getClosestCorner(const PosVector& vec) const;

	TilePtrList getTiles() const;
	EdgePtrList getEdges() const;
	CornerPtrList getCorners() const;
	double getRadius() const;

protected:
	void subdivideGrid();
	void createBaseGrid();
	EdgePtr createEdge(const CornerPtr& startPoint, const CornerPtr& endPoint);
	TilePtr createTile(const PosVector& pos, const EdgePtrList edgeLoop);
	TilePtr createTileFromSubdivision(const CornerPtr& baseCorner);
	void registerTileWithEdges(const TilePtr& tileptr);
	TilePtr createTile(const std::vector<PosVector>& cornerPoints);
	EdgePtrList createEdgeLoop(const std::vector<PosVector>& cornerPoints);

	double radius;

	TileMap tiles;
	CornerMap corners;
	EdgeMap edges;
};


#endif