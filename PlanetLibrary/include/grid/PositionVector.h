#pragma once

#include "../PlanetLibraryExports.h"
#include "boost/numeric/ublas/vector.hpp"
#include <unordered_map>
#include <map>
#include <memory>

#define PI 3.14159265359

typedef boost::numeric::ublas::vector<double> PosVector;

PLANET_LIB_API PosVector cross_product(const PosVector& lhs, const PosVector& rhs);

PLANET_LIB_API PosVector getAveragedVectorOnSphere(const std::vector<PosVector>& vectors, const double& radius);
PLANET_LIB_API PosVector getAveragedVector(const std::vector<PosVector>& vectors);

template <int percision = 2>
struct VecHash
{
	double cantorPair(const PosVector& key) const
	{
		assert(key.size() == 3);
		double x = key[0];
		double y = key[1];
		double z = key[2];
		double xy = std::sqrt(x*x + y*y);
		double theta = std::acos(x / xy);
		if (y<0)
		{
			theta += 180.0;
		}
		double phi = std::atan(z / xy) + 90.0;
		return 0.5*(theta + phi)*(theta + phi + 1) + phi;
	}

	std::size_t operator()(const PosVector& key) const
	{
		double _cantorPair = cantorPair(key);

		size_t maxSize = sizeof(unsigned int);
		if (maxSize < _cantorPair)
		{
			_cantorPair *= maxSize / (0.5*(360 + 180)*(360 + 180 + 1) + 180);
		}

		return size_t(std::ceil(_cantorPair * std::pow(10,percision)));
	}
};

template <int percision = 2>
struct VecCompare
{
	bool operator()(const PosVector& lhs, const PosVector& rhs) const
	{
		VecHash<percision> hasher;
		return hasher.cantorPair(lhs) < hasher.cantorPair(rhs);
	}
};

template<int percision = 2>
struct VecEquals
{
	unsigned int roundInt = unsigned int(std::pow(10, percision));
	bool operator()(const PosVector& lhs, const PosVector& rhs) const
	{
		bool equals = lhs.size() == rhs.size();
		for (size_t i = 0; i < lhs.size() && equals; i++)
		{
			equals = equals && int(std::round(lhs[i] * roundInt)) == int(std::round(rhs[i] * roundInt));
		}
		return equals;
	}
};

class Tile;
class Edge;
class Corner;

template<typename _ManagedType>
std::vector<std::shared_ptr<_ManagedType>> lockList(const std::vector<std::weak_ptr<_ManagedType>>& wPtrList)
{
	std::vector<std::shared_ptr<_ManagedType>> lockedPointers;
	std::transform(wPtrList.begin(), wPtrList.end(), std::back_inserter(lockedPointers),
		[](const std::weak_ptr<_ManagedType>& wPtr)->std::shared_ptr<_ManagedType>
	{
		return wPtr.lock();
	});
	return lockedPointers;
}

typedef std::shared_ptr<Tile> TilePtr;
typedef std::weak_ptr<Tile> TileWPtr;
typedef std::vector<TilePtr> TilePtrList;
typedef std::vector<TileWPtr> TileWPtrList;
typedef std::shared_ptr<Edge> EdgePtr;
typedef std::weak_ptr<Edge> EdgeWPtr;
typedef std::vector<EdgePtr> EdgePtrList;
typedef std::vector<EdgeWPtr> EdgeWPtrList;
typedef std::shared_ptr<Corner> CornerPtr;
typedef std::weak_ptr<Corner> CornerWPtr;
typedef std::vector<CornerPtr> CornerPtrList;
typedef std::vector<CornerWPtr> CornerWPtrList;

typedef std::unordered_map<PosVector, TilePtr, VecHash<2>,VecEquals<2>> TileUMap;
typedef std::unordered_map<PosVector, EdgePtr, VecHash<2>, VecEquals<2>> EdgeUMap;
typedef std::unordered_map<PosVector, CornerPtr, VecHash<2>, VecEquals<2>> CornerUMap;

typedef std::map<PosVector, TilePtr, VecCompare<2>> TileMap;
typedef std::map<PosVector, EdgePtr, VecCompare<2>> EdgeMap;
typedef std::map<PosVector, CornerPtr, VecCompare<2>> CornerMap;
