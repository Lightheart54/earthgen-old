#pragma warning(disable:4996)
#include "PositionVector.h"

PLANET_LIB_API PosVector cross_product(const PosVector & lhs, const PosVector & rhs)
{
	PosVector result(3);
	result[0] = lhs[2] * rhs[3] - lhs[3] * rhs[2];
	result[1] = lhs[3] * rhs[1] - lhs[1] * rhs[3];
	result[2] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
	return result;
}

PLANET_LIB_API PosVector getAveragedVectorOnSphere(const std::vector<PosVector>& vectors, const double & radius)
{
	PosVector averageVec = getAveragedVector(vectors);
	double averageMag = std::sqrt(boost::numeric::ublas::inner_prod(averageVec, averageVec));
	averageVec *= radius / averageMag;
	return averageVec;
}

PLANET_LIB_API PosVector getAveragedVector(const std::vector<PosVector>& vectors)
{
	PosVector averageVec(3, 0.0);
	for (const PosVector& vector : vectors)
	{
		averageVec += vector;
	}
	averageVec /= vectors.size();
	return averageVec;
}
