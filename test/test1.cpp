#define BOOST_TEST_MODULE My Test

#include <boost/test/included/unit_test.hpp>
#include "MVMCore.h"

BOOST_AUTO_TEST_CASE(first_test)
{
	MVMCore MVM;
	int i = 1;
	BOOST_TEST (i);
	BOOST_TEST(i == 2);
}
