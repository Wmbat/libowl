#include <epona_util/containers/flat_map.hpp>

#include <gtest/gtest.h>
#include <string>

struct copyable
{
   copyable() = default;
   explicit copyable(int i) : i(i) {}

   bool operator==(copyable const& other) const = default;

   int i = 0;
};

struct moveable
{
   moveable() = default;
   explicit moveable(int i) : i(i) {}
   moveable(moveable const& other) = delete;
   moveable(moveable&& other) { i = std::move(other.i); }

   moveable& operator=(moveable const& other) = delete;
   moveable& operator=(moveable&& other)
   {
      i = std::move(other.i);
      return *this;
   }

   int i = 0;
};

struct tiny_flat_avl_map_test : public testing::Test
{
   tiny_flat_avl_map_test() = default;
};

TEST_F(tiny_flat_avl_map_test, default_ctor)
{
   util::tiny_flat_map<int, int, 16> map{};
}

TEST_F(tiny_flat_avl_map_test, clear)
{
   util::tiny_flat_map<int, int, 16> map{};

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
   util::tiny_flat_map<int, int, 16> map{};

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

TEST_F(tiny_flat_avl_map_test, insert_rvalue_reference)
{
   util::tiny_flat_map<int, int, 16> map{};

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 16);

   auto [it_a, res_a] = map.insert({10, 10});

   EXPECT_EQ(res_a, true);
   EXPECT_EQ(it_a->first, 10);
   EXPECT_EQ(it_a->second, 10);
   EXPECT_EQ(it_a, map.begin());

   EXPECT_EQ(map.size(), 1);

   auto [it_a1, res_a1] = map.insert({10, 10});

   EXPECT_EQ(res_a1, false);
   EXPECT_EQ(it_a1->first, 10);
   EXPECT_EQ(it_a1->second, 10);
   EXPECT_EQ(it_a1, map.begin());

   EXPECT_EQ(map.size(), 1);

   auto [it_b, res_b] = map.insert({20, 20});

   EXPECT_EQ(res_b, true);
   EXPECT_EQ(it_b->first, 20);
   EXPECT_EQ(it_b->second, 20);
   EXPECT_EQ(it_b, map.begin() + 1);
   EXPECT_EQ(map.size(), 2);
}

TEST_F(tiny_flat_avl_map_test, insert_iterator_range)
{
   util::tiny_flat_map<int, int, 16> map{};

   {
      EXPECT_EQ(map.size(), 0);
      EXPECT_EQ(map.capacity(), 16);

      auto [it_a, res_a] = map.insert({10, 10});

      EXPECT_EQ(res_a, true);
      EXPECT_EQ(it_a->first, 10);
      EXPECT_EQ(it_a->second, 10);
      EXPECT_EQ(it_a, map.begin());

      EXPECT_EQ(map.size(), 1);

      auto [it_a1, res_a1] = map.insert({10, 10});

      EXPECT_EQ(res_a1, false);
      EXPECT_EQ(it_a1->first, 10);
      EXPECT_EQ(it_a1->second, 10);
      EXPECT_EQ(it_a1, map.begin());

      EXPECT_EQ(map.size(), 1);

      auto [it_b, res_b] = map.insert({20, 20});

      EXPECT_EQ(res_b, true);
      EXPECT_EQ(it_b->first, 20);
      EXPECT_EQ(it_b->second, 20);
      EXPECT_EQ(it_b, map.begin() + 1);
      EXPECT_EQ(map.size(), 2);
   }

   util::tiny_flat_map<int, int, 16> map_2{};

   map_2.insert(map.cbegin(), map.cend());
}

TEST_F(tiny_flat_avl_map_test, insert_initializer_list)
{
   util::tiny_flat_map<int, copyable, 2> map{};

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 2);

   map.insert({
      std::make_pair(3, copyable(3)),
      std::make_pair(1, copyable(1)),
      std::make_pair(2, copyable(2)),
   });

   EXPECT_EQ(map.size(), 3);
   EXPECT_EQ(map.capacity(), 5);

   for (int i = 0; const auto& [key, val] : map)
   {
      EXPECT_EQ(key, ++i);
      EXPECT_EQ(val.i, i);
   }
}

TEST_F(tiny_flat_avl_map_test, emplace)
{
   util::tiny_flat_map<int, std::string, 8> map{};

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 8);

   map.insert({
      std::make_pair(4, "fourth"),
      std::make_pair(1, "first"),
      std::make_pair(2, "second"),
   });

   EXPECT_EQ(map.size(), 3);
   EXPECT_EQ(map.capacity(), 8);

   {
      auto [it, res] = map.emplace(3, "third");

      EXPECT_EQ(it->first, 3);
      EXPECT_EQ(it->second, "third");
      EXPECT_EQ(res, true);
   }

   {
      auto [it, res] = map.emplace(3, "fourth");

      EXPECT_EQ(it->first, 3);
      EXPECT_EQ(it->second, "third");
      EXPECT_EQ(res, false);
   }

   EXPECT_EQ(map.size(), 4);
   EXPECT_EQ(map.capacity(), 8);
}

TEST_F(tiny_flat_avl_map_test, try_emplace)
{
   util::tiny_flat_map<int, std::string, 16> map{};

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 16);

   {
      auto [it, res] = map.try_emplace(1, 10, 'c');

      EXPECT_EQ(map.size(), 1);
      EXPECT_EQ(res, true);
      EXPECT_EQ(it->first, 1);
      EXPECT_EQ(it->second, "cccccccccc");
   }

   {
      auto [it, res] = map.try_emplace(1, "abc");

      EXPECT_EQ(map.size(), 1);
      EXPECT_EQ(res, false);
      EXPECT_EQ(it->first, 1);
      EXPECT_EQ(it->second, "cccccccccc");
   }

   {
      auto [it, res] = map.try_emplace(2, "test");

      EXPECT_EQ(map.size(), 2);
      EXPECT_EQ(res, true);
      EXPECT_EQ(it->first, 2);
      EXPECT_EQ(it->second, "test");
   }
}

TEST_F(tiny_flat_avl_map_test, erase)
{
   util::tiny_flat_map<int, copyable, 2> map{};

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 2);

   map.insert({
      std::make_pair(3, copyable(3)),
      std::make_pair(1, copyable(1)),
      std::make_pair(2, copyable(2)),
   });

   EXPECT_EQ(map.size(), 3);
   EXPECT_EQ(map.capacity(), 5);

   for (int i = 0; const auto& [key, val] : map)
   {
      EXPECT_EQ(key, ++i);
      EXPECT_EQ(val.i, i);
   }

   {
      auto it = map.erase(map.cbegin());

      EXPECT_EQ(map.size(), 2);
      EXPECT_EQ(it->first, 2);
      EXPECT_EQ(it->second.i, 2);
   }
   {
      auto it = map.erase(map.cend() - 1);

      EXPECT_EQ(map.size(), 1);
      EXPECT_EQ(map.end(), it);
   }
}

TEST_F(tiny_flat_avl_map_test, erase_iterator_range)
{
   util::tiny_flat_map<int, copyable, 2> map{};

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 2);

   map.insert({std::make_pair(3, copyable(3)), std::make_pair(1, copyable(1)),
      std::make_pair(2, copyable(2)), std::make_pair(7, copyable(7)),
      std::make_pair(5, copyable(5))});

   EXPECT_EQ(map.size(), 5);
   EXPECT_EQ(map.capacity(), 5);

   map.erase(map.cbegin(), map.cbegin() + 2);

   EXPECT_EQ(map.size(), 3);
   EXPECT_EQ(map.capacity(), 5);

   map.erase(map.cbegin(), map.cend());

   EXPECT_EQ(map.size(), 0);
   EXPECT_EQ(map.capacity(), 5);
}

TEST_F(tiny_flat_avl_map_test, count)
{
   util::tiny_flat_map<int, int, 16> map{};

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
   util::tiny_flat_map<int, int, 16> map{};

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
   util::tiny_flat_map<int, int, 16> map{};

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
   util::tiny_flat_map<int, int, 16> map{};

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
