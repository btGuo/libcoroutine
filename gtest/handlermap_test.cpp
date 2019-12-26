#include <gtest/gtest.h>
#include "../src/handlermap.h"

TEST(HandlerMapTest, InsertAndGet)
{
    co::HandlerMap<int *> map;
    int a = 10;
    int b = 11;
    map.insert(1, &a);
    map.insert(1134, &b);
    ASSERT_EQ(map.get(1), &a);
    ASSERT_EQ(map.get(1134), &b);
}
