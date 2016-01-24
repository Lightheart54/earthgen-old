#pragma warning(disable:4996)
#include "grid/grid.h"
#include <iostream>
#include <boost/numeric/ublas/io.hpp>

using boost::numeric::ublas::inner_prod;

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

template <typename _VectorType>
void cleanWPtrs(_VectorType& wPtrList)
{
	auto revIt = std::remove_if(wPtrList.begin(), wPtrList.end(), [](const typename _VectorType::value_type& wptr)->bool
	{
		return wptr.expired();
	});
	wPtrList.erase(revIt, wPtrList.end());
}

Grid Grid::createGridWithTileArea(const double & _radius, const double & desiredTileArea)
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
	cleanUpMemberWPtrLists();
	for (size_t i = 0; i < numSubdivisions; i++)
	{
		subdivideGrid();
		cleanUpMemberWPtrLists();
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

double Grid::getVolume() const
{
	double totalVolume = 0.0;
	for (const TilePtr& face : getTiles())
	{
		totalVolume += face->getEnclosedVolume();
	}
	return totalVolume;
}

double Grid::getSurfaceArea() const
{
	double totalSurfaceArea = 0.0;
	for (const TilePtr& face : getTiles())
	{
		totalSurfaceArea += face->getArea();
	}
	return totalSurfaceArea;
}

void Grid::subdivideGrid()
{
	TileUMap oldTiles = tiles;
	EdgeUMap oldEdges = edges;
	CornerUMap oldCorners = corners;
	tiles.clear();
	edges.clear();
	corners.clear();

	//first subdivide the old tiles, while we're at it keep track of the new corners that
	//will go with each of the old corners to make our new tiles
	std::unordered_map<CornerPtr, CornerPtrList> newTileCornerMap;
	for (const TileUMap::value_type& tilePair : oldTiles)
	{
		PosVector tileCenter = tilePair.first;
		PosVector tileCenterUV = getUnitVector(tileCenter);
		EdgePtrList tileEdges = tilePair.second->getEdges();
		CornerPtrList newCorners;
		for (const EdgePtr& tileEdge : tileEdges)
		{
			PosVector edgeCenter = tileEdge->getPosition();
			PosVector edgeCenterUV = getUnitVector(edgeCenter);
			PosVector edgeEnd = tileEdge->getEndPoints().front()->getPosition();
			PosVector cenMid = edgeCenter - tileCenter;
			double magCenMid = std::sqrt(inner_prod(cenMid, cenMid));
			PosVector cenMidUV = cenMid/magCenMid;
			PosVector cenEnd = edgeEnd - tileCenter;
			PosVector cenEndUV = getUnitVector(cenEnd);
			//the new half edge length must preserve the interior angle of tile for the edge
			//and also result in a corner that is the half length is the distance between the
			//line through the edge center and the origin and the new corner
			double sineHalfTileInteriorAngle = std::sqrt(1 - std::pow(inner_prod(cenMidUV, cenEndUV), 2));
			double cosineHalfGridInteriorAngle = inner_prod(tileCenterUV, edgeCenterUV);
			double newHalfEdgeLength = magCenMid / (sineHalfTileInteriorAngle + 1.0 / cosineHalfGridInteriorAngle);
			double projectedLength = newHalfEdgeLength / cosineHalfGridInteriorAngle;

			PosVector newCornerLoc = edgeCenter + -1 * cenMidUV*projectedLength;//cenMidUV points from the tile center to the edgeCenter
			CornerPtr newCorner = createCorner(newCornerLoc);
			newCorners.push_back(newCorner);
			for (const CornerPtr& oldCorner : tileEdge->getEndPoints())
			{
				if (newTileCornerMap.find(oldCorner) != newTileCornerMap.end())
				{
					newTileCornerMap[oldCorner].push_back(newCorner);
				}
				else
				{
					newTileCornerMap.insert({ oldCorner,{newCorner} });
				}
			}
		}
		//remake our old tile with its new corners
		createTile(newCorners);
	}

	//now make our new tiles
	for (const std::unordered_map<CornerPtr, CornerPtrList>::value_type& newTileCornerPair:newTileCornerMap)
	{
		createTile(newTileCornerPair.second);
	}
}

void Grid::getClosestNewCornerFromMidPoint(const CornerPtr& midPoint, CornerPtrList &sortedNewCorners,
	const PosVector& tileCenter, CornerPtrList &newCorners) const
{
	CornerPtrList connectedPoints = midPoint->getCorners();
	std::sort(connectedPoints.begin(), connectedPoints.end());
	std::unique(connectedPoints.begin(), connectedPoints.end());
	CornerPtrList connectedNewCorners;
	std::set_intersection(connectedPoints.begin(), connectedPoints.end(),
		sortedNewCorners.begin(), sortedNewCorners.end(), std::back_inserter(connectedNewCorners));
	PosVector vec1 = connectedNewCorners[0]->getPosition();
	PosVector vec2 = connectedNewCorners[1]->getPosition();
	vec1 -= tileCenter;
	vec2 -= tileCenter;
	double sqrdDist1 = inner_prod(vec1, vec1);
	double sqrdDist2 = inner_prod(vec2, vec2);
	if (sqrdDist1 < sqrdDist2)
	{
		newCorners.push_back(connectedNewCorners[0]);
	}
	else
	{
		newCorners.push_back(connectedNewCorners[1]);
	}
}

void Grid::createBaseGrid()
{
	//creating base dodecahedron
	double h = (std::sqrt(5) - 1) / 2.0;
	double x = 1 + h;
	double z = 1 - std::pow(h,2);
	double crossEdgeVertexLength = 2*z;
	double circumscribedRadius = crossEdgeVertexLength / 20 * sqrt(250 + 110*sqrt(5));
	double circumscribingRadius = sqrt(3.0);
	double pointMagnitudeRatio = radius / circumscribedRadius;
	x *= pointMagnitudeRatio;
	z *= pointMagnitudeRatio;
 
	std::vector<PosVector> vertexPos(20,PosVector(3));
	vertexPos[0][0] = 1 * pointMagnitudeRatio;
	vertexPos[0][1] = 1 * pointMagnitudeRatio;
	vertexPos[0][2] = 1 * pointMagnitudeRatio;
	vertexPos[1][0] = 1 * pointMagnitudeRatio;
	vertexPos[1][1] = 1 * pointMagnitudeRatio;
	vertexPos[1][2] = -1 * pointMagnitudeRatio;
	vertexPos[2][0] = 1 * pointMagnitudeRatio;
	vertexPos[2][1] = -1 * pointMagnitudeRatio;
	vertexPos[2][2] = 1 * pointMagnitudeRatio;
	vertexPos[3][0] = 1 * pointMagnitudeRatio;
	vertexPos[3][1] = -1 * pointMagnitudeRatio;
	vertexPos[3][2] = -1 * pointMagnitudeRatio;
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
#ifdef _DEBUG
	std::cout << "Starting Points:" << std::endl;
	for (const PosVector& startingPoint : vertexPos)
	{
		std::cout << startingPoint << std::endl;
	}
	std::cout << std::endl;
#endif

	for (const PosVector& pVec : vertexPos)
	{
		createCorner(pVec);
	}
	createTile({ vertexPos[0],	vertexPos[16],	vertexPos[2],	vertexPos[13],	vertexPos[12] });
	createTile({ vertexPos[1],	vertexPos[12],	vertexPos[13],	vertexPos[3],	vertexPos[18] });
	createTile({ vertexPos[0],	vertexPos[8],	vertexPos[4],	vertexPos[17],	vertexPos[16] });
	createTile({ vertexPos[6],	vertexPos[10],	vertexPos[2],	vertexPos[16],	vertexPos[17] });
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
#ifdef _DEBUG
	std::cout << "Looking For edge with Pos :" << newEdge->getPosition() << " Result: ";
#endif
	if (edges.find(newEdge->position) == edges.end())
	{
		edges.insert({ newEdge->getPosition(),newEdge });
#ifdef _DEBUG
		std::cout << "New Edge :" << startPoint->getPosition() << ", " << endPoint->getPosition() << std::endl;
#endif
	}
	else
	{
		newEdge = edges.at(newEdge->position);
#ifdef _DEBUG
		std::cout << "Retrieved Edge :" << startPoint->getPosition() << ", " << endPoint->getPosition() << std::endl;
#endif
	}
	return newEdge;
}

TilePtr Grid::createTile(const EdgePtrList& edgeLoop)
{
	TilePtr newTile = std::make_shared<Tile>(edgeLoop);
	tiles.insert({ newTile->getPosition(),newTile });
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
#ifdef _DEBUG
	std::cout << "Creating Edge Loop from Points:" << std::endl;
	for (const CornerPtr& corner: cornerPoints)
	{
		std::cout << corner->getPosition() << std::endl;
	}
#endif
	
	CornerPtr nextCorner = cornerPoints.front();
	CornerPtr startCorner = cornerPoints.front();
#ifdef _DEBUG
	CornerPtrList finalPointList;
#endif
	do
	{
		//find the two other closest corners
		PosVector currentPos = nextCorner->position;
#ifdef _DEBUG
        finalPointList.push_back(nextCorner);
		std::cout << "Current Pos:" << currentPos << std::endl;
#endif // _DEBUG
		std::sort(cornerPoints.begin(), cornerPoints.end(),
			[&currentPos](const CornerPtr& c1, const CornerPtr& c2)->bool
		{
			PosVector c1Pos = c1->position - currentPos;
			PosVector c2Pos = c2->position - currentPos;
			return inner_prod(c1Pos, c1Pos) < inner_prod(c2Pos, c2Pos);
		});
#ifdef _DEBUG
		std::cout << "Sort Results: " << std::endl;
		for (const CornerPtr& corner : cornerPoints)
		{
			std::cout << corner->getPosition() << std::endl;
		}
		std::cout << std::endl;
#endif


		CornerPtr option1 = cornerPoints[1]; // cornerPoint[0] should now be the current point
		CornerPtr option2 = cornerPoints[2]; // cornerPoint[0] should now be the current point
		PosVector c1Pos = option1->position - currentPos;
		PosVector c2Pos = option2->position - currentPos;
		PosVector c1Xc2 = cross_product(c1Pos, c2Pos);
		double orderIndicator = inner_prod(c1Xc2, currentPos);
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

#ifdef _DEBUG
	std::cout << "Final Edge Loop Points:" << std::endl;
	for (const CornerPtr& corner : finalPointList)
	{
		std::cout << corner->getPosition() << std::endl;
	}
#endif

	return edgeloop;
}

void Grid::cleanUpMemberWPtrLists() const
{
	for (const TileUMap::value_type& tilePair:tiles)
	{
		cleanWPtrs(tilePair.second->edges);
	}
	for (const EdgeUMap::value_type& edgePair : edges)
	{
		cleanWPtrs(edgePair.second->tiles);
		cleanWPtrs(edgePair.second->corners);
	}
	for (const CornerUMap::value_type& cornerPair : corners)
	{
		cleanWPtrs(cornerPair.second->edges);
	}
}
