#ifndef grid_h
#define grid_h

#include "../PlanetLibraryExports.h"
#include <map>
#include "tile.h"
#include "corner.h"
#include "edge.h"

class PLANET_LIB_API Grid
{
public:
	static Grid createGridWithTileArea(const double& _radius, const double& desiredTileArea);
	Grid(const double& _radius, const unsigned int& numSubdivisions);
	~Grid();

	TilePtr getTile(const PosVector& vec) const;
	EdgePtr getEdge(const PosVector& vec) const;
	CornerPtr getCorner(const PosVector& vec) const;

	TilePtrList getTiles() const;
	EdgePtrList getEdges() const;
	CornerPtrList getCorners() const;
	double getRadius() const;

	double getVolume() const;
	double getSurfaceArea() const;

protected:
	void subdivideGrid();
	void createBaseGrid();
	CornerPtr createCorner(const PosVector& pos);
	EdgePtr createEdge(const CornerPtr& startPoint, const CornerPtr& endPoint);
	EdgePtr createAndRegisterEdge(const CornerPtr& startPoint, const CornerPtr& endPoint);
	TilePtr createTile(const EdgePtrList& edgeLoop);
	void registerTileWithEdges(const TilePtr& tileptr);
	TilePtr createTile(const std::vector<PosVector>& cornerPoints);
	TilePtr createTile(const CornerPtrList& cornerPoints);
	EdgePtrList createEdgeLoop(const std::vector<PosVector>& cornerPoints);
	EdgePtrList createEdgeLoop(CornerPtrList cornerPoints);

	double radius;

	TileUMap tiles;
	CornerUMap corners;
	EdgeUMap edges;
};


#endif