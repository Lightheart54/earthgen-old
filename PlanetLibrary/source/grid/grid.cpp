#pragma warning(disable:4996)
#include "grid.h"

template <typename _MapType>
typename _MapType::mapped_type mapLookup(const _MapType& map,
	const typename _MapType::key_type& key,
	typename _MapType::mapped_type notFoundValue)
{
	_MapType::const_iterator mapIt = map.find(key);
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

TilePtr Grid::getTile(const PosVector& vec) const
{
	return mapLookup(tiles, vec, nullptr);
}

EdgePtr Grid::getEdge(const PosVector& vec) const
{
	return mapLookup(edges, vec, nullptr);
}

CornerPtr Grid::getCorner(const PosVector& vec) const
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
	TileUMap oldTiles = tiles;
	EdgeUMap oldEdges = edges;
	CornerUMap oldCorners = corners;
	tiles.clear();
	edges.clear();
	corners.clear();

	//first we take every edge we have and subdivide it to create our new corners
	//we need to store these so that they stay alive until the end of this function
	CornerUMap midPoints;
	EdgePtrList linkingEdges;
	for (const EdgeUMap::value_type& EdgePair : oldEdges)
	{
		CornerPtr edgeCenter = std::make_shared<Corner>(EdgePair.first);
		midPoints.insert({ EdgePair.first,edgeCenter });
		CornerPtr oldStart = EdgePair.second->getEndPoints().front();
		CornerPtr oldEnd = EdgePair.second->getEndPoints().back();
		//the new edges must have a length of the old edge length /sqrt(3)
		//add be perpendicular to the old edge
		PosVector halfEdge = oldEnd->position - edgeCenter->position;
		double halfEdgeLength = std::sqrt(boost::numeric::ublas::inner_prod(halfEdge, halfEdge));
		double newHalfEdgeLength = halfEdgeLength / std::sqrt(3);
		PosVector halfEdgeUV = halfEdge / halfEdgeLength;
		PosVector edgeCenterUV = getAveragedVectorOnSphere({ EdgePair.first }, 1);
		PosVector newEdgeUV = cross_product(edgeCenterUV, halfEdgeUV);
		PosVector newStartPos = EdgePair.first - newHalfEdgeLength * newEdgeUV;
		PosVector newEndPos = EdgePair.first + newHalfEdgeLength * newEdgeUV;

		//this should work out to be greater than then length of the radius of the inscribed sphere
		assert(radius < std::sqrt(boost::numeric::ublas::inner_prod(newStartPos, newStartPos)));

		//create the new corners
		CornerPtr newStart = createCorner(newStartPos);
		CornerPtr newEnd = createCorner(newEndPos);

		//now we're going to create linking edges for the purposes of discovering these edges later
		linkingEdges.push_back(createEdge(oldStart, edgeCenter));
		linkingEdges.push_back(createEdge(edgeCenter, oldEnd));
		linkingEdges.push_back(createEdge(newStart, edgeCenter));
		linkingEdges.push_back(createEdge(edgeCenter, newEnd));
	}
	//sort the linking edges, midPoints, and newCorners so that we can use them for set intersections
	std::sort(linkingEdges.begin(), linkingEdges.end());
	CornerPtrList sortedMidPoints = getMapValues(midPoints);
	std::sort(sortedMidPoints.begin(), sortedMidPoints.end());
	CornerPtrList sortedNewCorners = getMapValues(corners);
	std::sort(sortedNewCorners.begin(), sortedNewCorners.end());

	//now we have all of the new corners we need to construct our new faces
	//let's begin with the old corners, these are the easy ones
	for (const CornerUMap::value_type& cornerPair : oldCorners)
	{
		CornerPtr oldCorner = cornerPair.second;
		CornerPtrList connectedCorners = oldCorner->getCorners();
		std::sort(connectedCorners.begin(), connectedCorners.end());
		CornerPtrList connectedMidPoints;
		std::set_intersection(connectedCorners.begin(), connectedCorners.end(),
			sortedMidPoints.begin(), sortedMidPoints.end(), std::back_inserter(connectedMidPoints));
		CornerPtrList newTileCorners;
		for (const CornerPtr& midPoint : connectedMidPoints)
		{
			CornerPtrList midPointCorners = midPoint->getCorners();
			newTileCorners.insert(newTileCorners.end(), midPointCorners.begin(), midPointCorners.end());
		}
		std::sort(newTileCorners.begin(), newTileCorners.end());
		std::unique(newTileCorners.begin(), newTileCorners.end());
		CornerPtrList connectedNewCorners;
		std::set_intersection(newTileCorners.begin(), newTileCorners.end(),
			sortedNewCorners.begin(), sortedNewCorners.end(), std::back_inserter(connectedNewCorners));
		assert(connectedCorners.size() == 6); //this should be a hexagon
		createTile(connectedNewCorners);
	}

	//finally we need to rebuild the old tiles
	for (const TileUMap::value_type& tilePair : oldTiles)
	{
		//we need the new corner attached to each old edge's midpoint
		//that is closest the the tile center
		PosVector tileCenter = tilePair.first;
		EdgePtrList tileEdges = tilePair.second->getEdges();
		CornerPtrList newCorners;
		for (const EdgePtr& tileEdge : tileEdges)
		{
			CornerPtr midPoint = mapLookup(midPoints, tileEdge->getPosition(), nullptr);
			CornerPtrList connectedPoints = midPoint->getCorners();
			std::sort(connectedPoints.begin(), connectedPoints.end());
			CornerPtrList connectedNewCorners;
			std::set_intersection(connectedPoints.begin(), connectedPoints.end(),
				sortedNewCorners.begin(), sortedNewCorners.end(), std::back_inserter(connectedNewCorners));
			PosVector vec1 = connectedNewCorners[0]->getPosition();
			PosVector vec2 = connectedNewCorners[1]->getPosition();
			vec1 -= tileCenter;
			vec2 -= tileCenter;
			double sqrdDist1 = boost::numeric::ublas::inner_prod(vec1, vec1);
			double sqrdDist2 = boost::numeric::ublas::inner_prod(vec2, vec2);
			if (sqrdDist1 < sqrdDist2)
			{
				newCorners.push_back(connectedNewCorners[0]);
			}
			else
			{
				newCorners.push_back(connectedNewCorners[1]);
			}
		}
		//finally we can make the tile
		createTile(newCorners);
	}
}

void Grid::createBaseGrid()
{
	//creating base dodecahedron
	double h = (std::sqrt(5) + 1) / 2.0;
	double x = 1 + h;
	double z = 1 - h*h;
	double crossEdgeVertexLength = std::sqrt(x*x + z*z);

	double circumscribedRadius = 15 * radius / std::sqrt(75.0+30.0*std::sqrt(5));
	double cubeVectorRatio = circumscribedRadius / std::sqrt(3);
	double crossEdgeVertexRatio = circumscribedRadius / crossEdgeVertexLength;
	x *= crossEdgeVertexRatio;
	z *= crossEdgeVertexRatio;
 
	std::vector<PosVector> vertexPos(20,PosVector(3));
	vertexPos[0][0] = 1 * cubeVectorRatio;
	vertexPos[0][1] = 1 * cubeVectorRatio;
	vertexPos[0][2] = 1 * cubeVectorRatio;
	vertexPos[1][0] = 1 * cubeVectorRatio;
	vertexPos[1][1] = 1 * cubeVectorRatio;
	vertexPos[1][2] = -1 * cubeVectorRatio;
	vertexPos[2][0] = 1 * cubeVectorRatio;
	vertexPos[2][1] = -1 * cubeVectorRatio;
	vertexPos[2][2] = 1 * cubeVectorRatio;
	vertexPos[3][0] = 1 * cubeVectorRatio;
	vertexPos[3][1] = -1 * cubeVectorRatio;
	vertexPos[3][2] = -1 * cubeVectorRatio;
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
		createCorner(pVec);
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

CornerPtr Grid::createCorner(const PosVector & pos)
{
	CornerPtr newCorner = std::make_shared<Corner>(pos);
	corners.insert({ pos, newCorner });
	return newCorner;
}

EdgePtr Grid::createEdge(const CornerPtr & startPoint, const CornerPtr & endPoint)
{
	EdgePtr newEdge = std::make_shared<Edge>(startPoint, endPoint);
	startPoint->edges.push_back(newEdge);
	endPoint->edges.push_back(newEdge);
	return newEdge;
}

EdgePtr Grid::createAndRegisterEdge(const CornerPtr & startPoint, const CornerPtr & endPoint)
{
	EdgePtr newEdge = createEdge(startPoint, endPoint);
	if (edges.find(newEdge->position) == edges.end())
	{
		edges.insert({ newEdge->getPosition(),newEdge });
	}
	else
	{
		newEdge = edges.at(newEdge->position);
	}
	return newEdge;
}

TilePtr Grid::createTile(const EdgePtrList& edgeLoop)
{
	TilePtr newTile = std::make_shared<Tile>(edgeLoop);
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
	EdgePtrList edgeLoop = createEdgeLoop(cornerPoints);
	return createTile(edgeLoop);
}

TilePtr Grid::createTile(const CornerPtrList & cornerPoints)
{
	EdgePtrList edgeLoop = createEdgeLoop(cornerPoints);
	return createTile(edgeLoop);
}

EdgePtrList Grid::createEdgeLoop(const std::vector<PosVector>& cornerPoints)
{
	CornerPtrList corners;
	for (size_t i = 0; i < cornerPoints.size(); i++)
	{
		corners.push_back(getCorner(cornerPoints[i]));
	}
	return createEdgeLoop(corners);
}

EdgePtrList Grid::createEdgeLoop(CornerPtrList cornerPoints)
{
	EdgePtrList edgeloop;
	
	CornerPtr nextCorner = cornerPoints.front();
	CornerPtr startCorner = cornerPoints.front();
	do
	{
		//find the two other closest corners
		PosVector currentPos = nextCorner->position;
		std::sort(cornerPoints.begin(), cornerPoints.end(),
			[&currentPos](const CornerPtr& c1, const CornerPtr& c2)->bool
		{
			PosVector c1Pos = c1->position - currentPos;
			PosVector c2Pos = c2->position - currentPos;
			return boost::numeric::ublas::inner_prod(c1Pos, c1Pos) < boost::numeric::ublas::inner_prod(c2Pos, c2Pos);
		});

		CornerPtr option1 = cornerPoints[1]; // cornerPoint[0] should now be the current point
		CornerPtr option2 = cornerPoints[2]; // cornerPoint[0] should now be the current point
		PosVector c1Pos = option1->position - currentPos;
		PosVector c2Pos = option2->position - currentPos;
		PosVector c1Xc2 = cross_product(c1Pos, c2Pos);
		double orderIndicator = boost::numeric::ublas::inner_prod(c1Xc2, currentPos);
		//if the orderIndicator is negative, it indicates that c1Xc2 is in the opposite
		//direction of the position vector for the current point, which we don't want
		//therefore option2 is the next point, otherwise option1 is the correct choice
		if (orderIndicator > 0)
		{
			edgeloop.push_back(createAndRegisterEdge(nextCorner, option1));
			nextCorner = option1;
		}
		else
		{
			edgeloop.push_back(createAndRegisterEdge(nextCorner, option2));
			nextCorner = option2;
		}

	} while (nextCorner != startCorner);

	return edgeloop;
}
