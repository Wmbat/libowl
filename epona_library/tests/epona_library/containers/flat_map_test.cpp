#include <epona_library/allocators/pool_allocator.hpp>
#include <epona_library/containers/flat_map.hpp>

struct tiny_flat_avl_map_test : public testing::Test
{
   tiny_flat_avl_map_test() :
      pool_allocator({.pool_count = 2, .pool_size = 2048}), secondary_allocator({.pool_count = 2, .pool_size = 2048})
   {}

   ESL::pool_allocator pool_allocator;
   ESL::pool_allocator secondary_allocator;
};

TEST_F(tiny_flat_avl_map_test, default_ctor)
{
   ESL::tiny_flat_map<int, int, 16, ESL::pool_allocator> map(&pool_allocator);
}

TEST_F(tiny_flat_avl_map_test, clear)
{
   ESL::tiny_flat_map<int, int, 16, ESL::pool_allocator> map(&pool_allocator);

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 16);

   std::pair a = {10, 10};

   auto [it_a, res_a] = map.insert(a);

   EXPECT_EQ(res_a, true);
   EXPECT_EQ(*it_a, a);
   EXPECT_EQ(it_a, map.begin());

   EXPECT_EQ(map.size(), 1);

   auto [it_a1, res_a1] = map.insert(a);

   EXPECT_EQ(res_a1, false);
   EXPECT_EQ(*it_a1, a);
   EXPECT_EQ(it_a1, map.begin());

   EXPECT_EQ(map.size(), 1);

   std::pair b = {20, 20};
   auto [it_b, res_b] = map.insert(b);

   EXPECT_EQ(res_b, true);
   EXPECT_EQ(*it_b, b);
   EXPECT_EQ(it_b, map.begin() + 1);
   EXPECT_EQ(map.size(), 2);

   map.clear();

   EXPECT_EQ(map.size(), 0);
}

TEST_F(tiny_flat_avl_map_test, insert_lvalue_reference)
{
   ESL::tiny_flat_map<int, int, 16, ESL::pool_allocator> map(&pool_allocator);

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 16);

   std::pair a = {10, 10};

   auto [it_a, res_a] = map.insert(a);

   EXPECT_EQ(res_a, true);
   EXPECT_EQ(*it_a, a);
   EXPECT_EQ(it_a, map.begin());

   EXPECT_EQ(map.size(), 1);

   auto [it_a1, res_a1] = map.insert(a);

   EXPECT_EQ(res_a1, false);
   EXPECT_EQ(*it_a1, a);
   EXPECT_EQ(it_a1, map.begin());

   EXPECT_EQ(map.size(), 1);

   std::pair b = {20, 20};
   auto [it_b, res_b] = map.insert(b);

   EXPECT_EQ(res_b, true);
   EXPECT_EQ(*it_b, b);
   EXPECT_EQ(it_b, map.begin() + 1);
   EXPECT_EQ(map.size(), 2);
}

TEST_F(tiny_flat_avl_map_test, count)
{
   ESL::tiny_flat_map<int, int, 16, ESL::pool_allocator> map(&pool_allocator);

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 16);

   std::pair a = {10, 10};

   auto [it_a, res_a] = map.insert(a);

   EXPECT_EQ(res_a, true);
   EXPECT_EQ(*it_a, a);
   EXPECT_EQ(it_a, map.begin());

   EXPECT_EQ(map.size(), 1);

   auto [it_a1, res_a1] = map.insert(a);

   EXPECT_EQ(res_a1, false);
   EXPECT_EQ(*it_a1, a);
   EXPECT_EQ(it_a1, map.begin());

   EXPECT_EQ(map.size(), 1);

   std::pair b = {20, 20};
   auto [it_b, res_b] = map.insert(b);

   EXPECT_EQ(res_b, true);
   EXPECT_EQ(*it_b, b);
   EXPECT_EQ(it_b, map.begin() + 1);
   EXPECT_EQ(map.size(), 2);

   EXPECT_EQ(map.count(20), 1);
   EXPECT_EQ(map.count(0), 0);
}

TEST_F(tiny_flat_avl_map_test, find)
{
   ESL::tiny_flat_map<int, int, 16, ESL::pool_allocator> map(&pool_allocator);

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 16);

   std::pair a = {10, 10};

   auto [it_a, res_a] = map.insert(a);

   EXPECT_EQ(res_a, true);
   EXPECT_EQ(*it_a, a);
   EXPECT_EQ(it_a, map.begin());

   EXPECT_EQ(map.size(), 1);

   auto [it_a1, res_a1] = map.insert(a);

   EXPECT_EQ(res_a1, false);
   EXPECT_EQ(*it_a1, a);
   EXPECT_EQ(it_a1, map.begin());

   EXPECT_EQ(map.size(), 1);

   std::pair b = {20, 20};
   auto [it_b, res_b] = map.insert(b);

   EXPECT_EQ(res_b, true);
   EXPECT_EQ(*it_b, b);
   EXPECT_EQ(it_b, map.begin() + 1);
   EXPECT_EQ(map.size(), 2);

   auto it_1 = map.find(10);

   EXPECT_EQ(it_1, map.begin());
   EXPECT_EQ(*it_1, a);

   it_1->second = 30;

   EXPECT_EQ(map.begin()->second, 30);

   auto it_2 = map.find(30);

   EXPECT_EQ(it_2, map.end());
}

TEST_F(tiny_flat_avl_map_test, find_const)
{
   ESL::tiny_flat_map<int, int, 16, ESL::pool_allocator> map(&pool_allocator);

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 16);

   std::pair a = {10, 10};

   auto [it_a, res_a] = map.insert(a);

   EXPECT_EQ(res_a, true);
   EXPECT_EQ(*it_a, a);
   EXPECT_EQ(it_a, map.begin());

   EXPECT_EQ(map.size(), 1);

   auto [it_a1, res_a1] = map.insert(a);

   EXPECT_EQ(res_a1, false);
   EXPECT_EQ(*it_a1, a);
   EXPECT_EQ(it_a1, map.begin());

   EXPECT_EQ(map.size(), 1);

   std::pair b = {20, 20};
   auto [it_b, res_b] = map.insert(b);

   EXPECT_EQ(res_b, true);
   EXPECT_EQ(*it_b, b);
   EXPECT_EQ(it_b, map.begin() + 1);
   EXPECT_EQ(map.size(), 2);

   auto it_1 = map.find(10);

   EXPECT_EQ(it_1, map.begin());
   EXPECT_EQ(*it_1, a);

   auto it_2 = map.find(30);

   EXPECT_EQ(it_2, map.end());
}

TEST_F(tiny_flat_avl_map_test, contains)
{
   ESL::tiny_flat_map<int, int, 16, ESL::pool_allocator> map(&pool_allocator);

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 16);

   std::pair a = {10, 10};

   auto [it_a, res_a] = map.insert(a);

   EXPECT_EQ(res_a, true);
   EXPECT_EQ(*it_a, a);
   EXPECT_EQ(it_a, map.begin());

   EXPECT_EQ(map.size(), 1);

   auto [it_a1, res_a1] = map.insert(a);

   EXPECT_EQ(res_a1, false);
   EXPECT_EQ(*it_a1, a);
   EXPECT_EQ(it_a1, map.begin());

   EXPECT_EQ(map.size(), 1);

   std::pair b = {20, 20};
   auto [it_b, res_b] = map.insert(b);

   EXPECT_EQ(res_b, true);
   EXPECT_EQ(*it_b, b);
   EXPECT_EQ(it_b, map.begin() + 1);
   EXPECT_EQ(map.size(), 2);

   EXPECT_EQ(map.contains(20), true);
   EXPECT_EQ(map.contains(40), false);
}
