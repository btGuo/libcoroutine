#include <gtest/gtest.h>
#include "../src/list.h"
using namespace std;

class Integer:public co::ListHook<Integer>
{
public:
    Integer(){}
    Integer(int val):m_val(val){}
    int getVal() { return m_val; }
private:
    int m_val{0};
};

TEST(ListTest, PushBackAndPop)
{
    Integer a(10);
    Integer b(11);
    co::List<Integer> list;
    list.push_back(&a);
    list.push_back(&b);
    
    Integer *num = list.pop_front();
    ASSERT_EQ(num->getVal(), 10);
    num = list.pop_front();
    ASSERT_EQ(num->getVal(), 11);
}

TEST(ListTest, PushFrontAndPop)
{
    Integer a(10);
    Integer b(11);
    co::List<Integer> list;
    list.push_front(&a);
    list.push_front(&b);

    Integer *num = list.pop_front();
    ASSERT_EQ(num->getVal(), 11);
    num = list.pop_front();
    ASSERT_EQ(num->getVal(), 10);
}

TEST(ListTest, Size)
{
    Integer a(10);
    Integer b(11);
    co::List<Integer> list;

    ASSERT_EQ(list.size(), 0);

    list.push_front(&a);
    list.push_front(&b);

    ASSERT_EQ(list.size(), 2);

    list.pop_front();
    ASSERT_EQ(list.size(), 1);
}

