#pragma warning(disable:4996)
#include "grid.h"

template <typename _MapType>
typename _MapType::mapped_type mapLookup(const _MapType& map,
	const typename _MapType::key_type& key,
	typename _MapType::mapped_type notFoundValue)
{
	_MapType::const_iterator mapIt = map.lower_bound(key);
	if (mapIt != map.end())
	{
		return (*mapIt).second;
	}
	return notFoundValue;
}

template<typename _MapType>
std::vector<typename _MapType::mapped_type> getMapValues(const _MapType& map)
{
	std::vector<typename _MapType::mapped_type> values;
	std::transform(map.begin(), map.end(), std::back_inserter(values),
		[](const typename _MapType::value_type & keyValuePair)->_MapType::mapped_type
	{
		return keyValuePair.second;
	});
	return values;
}

Grid Grid::createGridWithEdgeLength(const double & _radius, const double & desiredTileArea)
{
	double sphereSurfaceArea = 4 * PI * _radius*_radius;
	double numberOfTiles = sphereSurfaceArea / desiredTileArea;
	unsigned int numSubdivision = 0;
	if (numberOfTiles > 12)
	{
		unsigned int numVert = 20;
		numberOfTiles -= 2;
		numberOfTiles /= 10;
		numSubdivision = unsigned int (std::round(std::log(numberOfTiles) / std::log(3)));
	}
	return Grid(_radius, numSubdivision);
}

Grid::Grid(const double & _radius, const unsigned int & numSubdivisions) : radius(_radius)
{
	createBaseGrid();
	for (size_t i = 0; i < numSubdivisions; i++)
	{
		subdivideGrid();
	}
}

Grid::~Grid()
{
}

TilePtr Grid::getClosestTile(const PosVector& vec) const
{
	return mapLookup(tiles, vec, nullptr);
}

EdgePtr Grid::getClosestEdge(const PosVector& vec) const
{
	return mapLookup(edges, vec, nullptr);
}

CornerPtr Grid::getClosestCorner(const PosVector& vec) const
{
	return mapLookup(corners, vec, nullptr);
}

TilePtrList Grid::getTiles() const
{
	return getMapValues(tiles);
}

EdgePtrList Grid::getEdges() const
{
	return getMapValues(edges);
}

CornerPtrList Grid::getCorners() const
{
	return getMapValues(corners);
}

double Grid::getRadius() const
{
	return radius;
}

void Grid::subdivideGrid()
{
}

void Grid::createBaseGrid()
{
	double h = (std::sqrt(5) + 1) / 2.0;
	double x = 1 + h;
	double z = 1 - h*h;
	double ceVertexLength = std::sqrt(x*x + z*z);
	x *= radius / ceVertexLength;
	z *= radius / ceVertexLength;
 
	std::vector<PosVector> vertexPos(20,PosVector(3));
	vertexPos[0][0] = radius;
	vertexPos[0][1] = radius;
	vertexPos[0][2] = radius;
	vertexPos[1][0] = radius;
	vertexPos[1][1] = radius;
	vertexPos[1][2] = -radius;
	vertexPos[2][0] = radius;
	vertexPos[2][1] = -radius;
	vertexPos[2][2] = radius;
	vertexPos[3][0] = radius;
	vertexPos[3][1] = -radius;
	vertexPos[3][2] = -radius;
	vertexPos[4] = -vertexPos[3];
	vertexPos[5] = -vertexPos[2];
	vertexPos[6] = -vertexPos[1];
	vertexPos[7] = -vertexPos[0];
	vertexPos[8][0] = 0.0;
	vertexPos[8][1] = x;
	vertexPos[8][2] = z;
	vertexPos[9][0] = 0.0;
	vertexPos[9][1] = x;
	vertexPos[9][2] = -z; 
	vertexPos[10] = -vertexPos[9];
	vertexPos[11] = -vertexPos[8];
	vertexPos[12][0] = x;
	vertexPos[12][1] = z;
	vertexPos[12][2] = 0.0;
	vertexPos[13][0] = x;
	vertexPos[13][1] = -z;
	vertexPos[13][2] = 0.0;
	vertexPos[14] = -vertexPos[13];
	vertexPos[15] = -vertexPos[12];
	vertexPos[16][0] = z;
	vertexPos[16][1] = 0.0;
	vertexPos[16][2] = x;
	vertexPos[17][0] = -z;
	vertexPos[17][1] = 0.0;
	vertexPos[17][2] = x;
	vertexPos[18] = -vertexPos[17];
	vertexPos[19] = -vertexPos[16];

	for (const PosVector& pVec : vertexPos)
	{
		corners.insert({ pVec,std::make_shared<Corner>(pVec) });
	}
	createTile({ vertexPos[0],	vertexPos[16],	vertexPos[2],	vertexPos[13],	vertexPos[12] });
	createTile({ vertexPos[1],	vertexPos[12],	vertexPos[13],	vertexPos[3],	vertexPos[18] });
	createTile({ vertexPos[0],	vertexPos[8],	vertexPos[4],	vertexPos[17],	vertexPos[16] });
	createTile({ vertexPos[6],	vertexPos[10],	vertexPos[1],	vertexPos[16],	vertexPos[17] });
	createTile({ vertexPos[5],	vertexPos[14],	vertexPos[4],	vertexPos[8],	vertexPos[9] });
	createTile({ vertexPos[1],	vertexPos[9],	vertexPos[8],	vertexPos[0],	vertexPos[12] });
	createTile({ vertexPos[3],	vertexPos[13],	vertexPos[2],	vertexPos[10],	vertexPos[11] });
	createTile({ vertexPos[7],	vertexPos[11],	vertexPos[10],	vertexPos[6],	vertexPos[15] });
	createTile({ vertexPos[5],	vertexPos[9],	vertexPos[1],	vertexPos[18],	vertexPos[19] });
	createTile({ vertexPos[7],	vertexPos[19],	vertexPos[18],	vertexPos[3],	vertexPos[11] });
	createTile({ vertexPos[5],	vertexPos[19],	vertexPos[7],	vertexPos[15],	vertexPos[14] });
	createTile({ vertexPos[4],	vertexPos[14],	vertexPos[15],	vertexPos[6],	vertexPos[17] });
}

EdgePtr Grid::createEdge(const CornerPtr & startPoint, const CornerPtr & endPoint)
{
	EdgePtr newEdge = std::make_shared<Edge>(startPoint, endPoint);
	startPoint->edges.push_back(newEdge);
	endPoint->edges.push_back(newEdge);
	edges.insert({ newEdge->getPosition(),newEdge });
	return newEdge;
}

TilePtr Grid::createTile(const PosVector & pos, const EdgePtrList edgeLoop)
{
	TilePtr newTile = std::make_shared<Tile>(pos, edgeLoop);
	registerTileWithEdges(newTile);
	return newTile;
}

TilePtr Grid::createTileFromSubdivision(const CornerPtr & baseCorner)
{
	TilePtr newTile = Tile::createOnSubdividedGrid(baseCorner);
	registerTileWithEdges(newTile);
	return newTile;
}

void Grid::registerTileWithEdges(const TilePtr & tileptr)
{
	EdgePtrList tileEdges = tileptr->getEdges();
	for (const EdgePtr& edge : tileEdges)
	{
		edge->tiles.push_back(tileptr);
	}
}

TilePtr Grid::createTile(const std::vector<PosVector>& cornerPoints)
{
	PosVector tilePos = getAveragedVectorOnSphere(cornerPoints, radius);
	EdgePtrList edgeLoop = createEdgeLoop(cornerPoints);
	return createTile(tilePos, edgeLoop);
}

EdgePtrList Grid::createEdgeLoop(const std::vector<PosVector>& cornerPoints)
{
	EdgePtrList edgeloop;
	for (size_t i = 0; i < cornerPoints.size(); i++)
	{
		size_t nextPoint = i < cornerPoints.size() - 1 ? i + 1 : 0;
		edgeloop.push_back(createEdge(getClosestCorner(cornerPoints[i]), getClosestCorner(cornerPoints[i])));
	}
	return edgeloop;
}
