#include "stdafx.h"
#include "grid/grid.h"

BOOST_AUTO_TEST_SUITE(GridTestSuite)

BOOST_AUTO_TEST_CASE(BaseGridTest)
{
	Grid testGrid(1.0, 0);

	BOOST_CHECK(testGrid.getTiles().size() == 12);
	BOOST_CHECK(testGrid.getEdges().size() == 30);
	BOOST_CHECK(testGrid.getCorners().size() == 20);
}

BOOST_AUTO_TEST_CASE(SubdivideOnceTest)
{
	Grid testGrid(1.0, 1);

	BOOST_CHECK(testGrid.getTiles().size() == 32);
	BOOST_CHECK(testGrid.getEdges().size() == 90);
	BOOST_CHECK(testGrid.getCorners().size() == 60);
}

BOOST_AUTO_TEST_SUITE_END()