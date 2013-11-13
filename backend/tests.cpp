#include "gtest/gtest.h"

using namespace std;

TEST(RWLock, foo) {
	int bar = 1;
    EXPECT_EQ(1, bar);
}

GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}