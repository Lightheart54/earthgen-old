#include "stdafx.h"
#include "grid/grid.h"

BOOST_AUTO_TEST_SUITE(GridTestSuite)

BOOST_AUTO_TEST_CASE(BaseGridTest)
{
	Grid testGrid(1.0, 0);

	BOOST_CHECK(testGrid.getTiles().size() == 12);
	BOOST_CHECK(testGrid.getEdges().size() == 30);
	BOOST_CHECK(testGrid.getCorners().size() == 20);

	double expectedApproximateSurfaceArea = std::round(16.65087309 * 10000) / 10000.0;
	double calculatedSurfaceArea = testGrid.getSurfaceArea();
	double roundedSurfaceArea = std::round(calculatedSurfaceArea * 10000) / 10000.0;
	BOOST_CHECK(roundedSurfaceArea == expectedApproximateSurfaceArea);

	double expectedApproximateVolume = std::round(5.550291029 * 10000) / 10000.0;
	double calculatedVolume = testGrid.getVolume();
	double roundedVolume = std::round(calculatedVolume * 10000) / 10000.0;
	BOOST_CHECK(roundedVolume == expectedApproximateVolume);
}

BOOST_AUTO_TEST_CASE(SubdivideOnceTest)
{
	Grid testGrid(1.0, 1);

	BOOST_CHECK(testGrid.getTiles().size() == 32);
	BOOST_CHECK(testGrid.getEdges().size() == 90);
	BOOST_CHECK(testGrid.getCorners().size() == 60);
	
	double sideLength = 2 * 0.350515589;

	double expectedApproximateSurfaceArea = std::pow(sideLength, 2) * (30.0*std::sqrt(3)+15.0*std::sqrt(1+2.0/std::sqrt(5)));
	expectedApproximateSurfaceArea = std::round(expectedApproximateSurfaceArea * 10000) / 10000.0;
	double calculatedSurfaceArea = testGrid.getSurfaceArea();
	double roundedSurfaceArea = std::round(calculatedSurfaceArea * 10000) / 10000.0;
	BOOST_CHECK(roundedSurfaceArea == expectedApproximateSurfaceArea);

	double expectedApproximateVolume = std::pow(sideLength, 3) * 1.25*(125+43.0*std::sqrt(5));
	expectedApproximateVolume = std::round(expectedApproximateVolume * 10000) / 10000.0;
	double calculatedVolume = testGrid.getVolume();
	double roundedVolume = std::round(calculatedVolume * 10000) / 10000.0;
	BOOST_CHECK(roundedVolume == expectedApproximateVolume);
}

BOOST_AUTO_TEST_SUITE_END()