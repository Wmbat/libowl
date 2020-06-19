/**
 * MIT License
 *
 * Copyright (c) 2020 Wmbat
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <epona_core/containers/dynamic_array.hpp>
#include <epona_core/detail/iterators/random_access_iterator.hpp>

#include <gtest/gtest.h>

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

struct tiny_dynamic_array_test : public testing::Test
{
   tiny_dynamic_array_test() = default;
};

TEST_F(tiny_dynamic_array_test, default_ctor)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);
   }

   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);
   }
}

TEST_F(tiny_dynamic_array_test, ctor_count)
{
   {
      core::tiny_dynamic_array<copyable, 0> vec{10};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 0);
      }
   }
   {
      core::tiny_dynamic_array<copyable, 5> vec{5};

      EXPECT_EQ(vec.size(), 5);
      EXPECT_EQ(vec.capacity(), 5);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 0);
      }
   }
}

TEST_F(tiny_dynamic_array_test, ctor_count_values)
{
   {
      core::tiny_dynamic_array<copyable, 0> vec{10, copyable{5}};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 5);
      }
   }
   {
      core::tiny_dynamic_array<copyable, 5> vec{5, copyable{20}};

      EXPECT_EQ(vec.size(), 5);
      EXPECT_EQ(vec.capacity(), 5);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 20);
      }
   }
}

TEST_F(tiny_dynamic_array_test, ctor_range)
{
   {
      core::tiny_dynamic_array<copyable, 0> vec{10, copyable{5}};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 5);
      }

      core::tiny_dynamic_array<copyable, 5> vec2{vec.begin(), vec.end()};

      EXPECT_EQ(vec2.size(), 10);
      EXPECT_EQ(vec2.capacity(), 11);

      for (auto& val : vec2)
      {
         EXPECT_EQ(val.i, 5);
      }
   }
}

TEST_F(tiny_dynamic_array_test, ctor_initializer_list)
{
   {
      core::tiny_dynamic_array<copyable, 10> vec{{copyable{1}, copyable{1}, copyable{1}}};

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 10);

      for (const auto& c : vec)
      {
         EXPECT_EQ(c.i, 1);
      }
   }
}

TEST_F(tiny_dynamic_array_test, ctor_copy)
{
   {
      core::tiny_dynamic_array<copyable, 0> vec{10, copyable{5}};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 5);
      }

      core::tiny_dynamic_array<copyable, 0> vec2{vec};

      EXPECT_EQ(vec2.size(), 10);
      EXPECT_EQ(vec2.capacity(), 10);

      for (auto& val : vec2)
      {
         EXPECT_EQ(val.i, 5);
      }
   }
}

TEST_F(tiny_dynamic_array_test, ctor_move)
{
   {
      core::tiny_dynamic_array<copyable, 0> vec{10, copyable{5}};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 5);
      }

      core::tiny_dynamic_array<copyable, 0> vec2{std::move(vec)};

      EXPECT_EQ(vec2.size(), 10);
      EXPECT_EQ(vec2.capacity(), 10);

      for (auto& val : vec2)
      {
         EXPECT_EQ(val.i, 5);
      }

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);
   }
   {
      core::tiny_dynamic_array<copyable, 10> vec{10, copyable{5}};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 5);
      }

      core::tiny_dynamic_array<copyable, 10> vec2{std::move(vec)};

      EXPECT_EQ(vec2.size(), 10);
      EXPECT_EQ(vec2.capacity(), 10);

      for (auto& val : vec2)
      {
         EXPECT_EQ(val.i, 5);
      }

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 10);
   }
}

TEST_F(tiny_dynamic_array_test, copy_assignment_operator)
{
   {
      core::tiny_dynamic_array<copyable, 0> vec{10, copyable{5}};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 5);
      }

      core::tiny_dynamic_array<copyable, 0> vec2 = vec;

      EXPECT_EQ(vec2.size(), 10);
      EXPECT_EQ(vec2.capacity(), 10);

      for (auto& val : vec2)
      {
         EXPECT_EQ(val.i, 5);
      }
   }
}

TEST_F(tiny_dynamic_array_test, move_assignment_operator)
{
   {
      core::tiny_dynamic_array<copyable, 0> vec{10, copyable{5}};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 5);
      }

      decltype(vec) vec2 = std::move(vec);

      EXPECT_EQ(vec2.size(), 10);
      EXPECT_EQ(vec2.capacity(), 10);

      for (auto& val : vec2)
      {
         EXPECT_EQ(val.i, 5);
      }

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);
   }
}

TEST_F(tiny_dynamic_array_test, equality_operator)
{
   {
      core::tiny_dynamic_array<copyable, 0> vec{10, copyable{5}};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 5);
      }

      core::tiny_dynamic_array<copyable, 0> vec2 = vec;

      EXPECT_EQ(vec2.size(), 10);
      EXPECT_EQ(vec2.capacity(), 10);

      for (auto& val : vec2)
      {
         EXPECT_EQ(val.i, 5);
      }

      if (vec != vec2)
      {
         FAIL();
      }
   }
   {
      core::tiny_dynamic_array<copyable, 10> vec{10, copyable{5}};

      EXPECT_EQ(vec.size(), 10);
      EXPECT_EQ(vec.capacity(), 10);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 5);
      }

      decltype(vec) vec2 = std::move(vec);

      EXPECT_EQ(vec2.size(), 10);
      EXPECT_EQ(vec2.capacity(), 10);

      for (auto& val : vec2)
      {
         EXPECT_EQ(val.i, 5);
      }

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 10);

      if (vec == vec2)
      {
         FAIL();
      }
   }
}

TEST_F(tiny_dynamic_array_test, assign_n_values)
{
   {
      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      auto it_one = vec.insert(vec.cbegin(), copyable{1});

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_one->i, 1);
      EXPECT_EQ(vec.begin()->i, 1);

      auto it_two = vec.insert(vec.cend(), copyable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_two->i, 2);
      EXPECT_EQ((++vec.begin())->i, 2);

      vec.assign(4, copyable{0});

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 5);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 0);
      }
   }
   {
      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      vec.assign(2, copyable{0});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 0);
      }
   }
}

TEST_F(tiny_dynamic_array_test, assign_range)
{
   {
      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      auto it_one = vec.insert(vec.cbegin(), copyable{1});

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_one->i, 1);
      EXPECT_EQ(vec.begin()->i, 1);

      auto it_two = vec.insert(vec.cend(), copyable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_two->i, 2);
      EXPECT_EQ((++vec.begin())->i, 2);

      vec.assign(4, copyable{0});

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 5);

      for (auto& val : vec)
      {
         EXPECT_EQ(val.i, 0);
      }

      core::tiny_dynamic_array<copyable, 10> vec2{};

      vec2.assign(vec.begin(), vec.end());

      EXPECT_EQ(vec2.size(), 4);
      EXPECT_EQ(vec2.capacity(), 10);

      for (auto& val : vec2)
      {
         EXPECT_EQ(val.i, 0);
      }
   }
}

TEST_F(tiny_dynamic_array_test, assign_initializer_list)
{
   {
      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      vec.assign({copyable{0}, copyable{1}});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);

      EXPECT_EQ(vec[0].i, 0);
      EXPECT_EQ(vec[1].i, 1);

      vec.assign({copyable{3}, copyable{2}, copyable{1}});

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 5);

      EXPECT_EQ(vec[0].i, 3);
      EXPECT_EQ(vec[1].i, 2);
      EXPECT_EQ(vec[2].i, 1);
   }
}

TEST_F(tiny_dynamic_array_test, clear)
{
   {
      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      auto it_one = vec.insert(vec.cbegin(), copyable{1});

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_one->i, 1);
      EXPECT_EQ(vec.begin()->i, 1);

      auto it_two = vec.insert(vec.cend(), copyable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_two->i, 2);
      EXPECT_EQ((++vec.begin())->i, 2);

      auto it_three = vec.insert(vec.cend() - 1, copyable{3});

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ(it_three->i, 3);
      EXPECT_EQ((++vec.begin())->i, 3);

      auto it_four = vec.insert(vec.cbegin(), copyable{4});

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ(it_four->i, 4);
      EXPECT_EQ(vec.begin()->i, 4);

      EXPECT_EQ(vec[0].i, 4);
      EXPECT_EQ(vec[1].i, 1);
      EXPECT_EQ(vec[2].i, 3);
      EXPECT_EQ(vec[3].i, 2);

      vec.clear();

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 5);
   }
}

TEST_F(tiny_dynamic_array_test, insert_lvalue_ref)
{
   {
      int one = 1;
      int two = 2;
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      auto it_one = vec.insert(vec.cbegin(), one);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*it_one, one);
      EXPECT_EQ(*vec.begin(), one);

      auto it_two = vec.insert(vec.cbegin(), two);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_two, two);
      EXPECT_EQ(*vec.begin(), two);
      EXPECT_EQ(*(++vec.begin()), one);

      auto it_three = vec.insert(vec.cend() - 1, one);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_three, one);
      EXPECT_EQ(*(vec.end() - 1), one);

      for (int i = 2; auto& val : vec)
      {
         EXPECT_EQ(val, i);

         if (i != 1)
         {
            --i;
         }
      }
   }

   {
      copyable one{1};
      copyable two{2};
      copyable three{3};
      copyable four{4};

      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      auto it_one = vec.insert(vec.cbegin(), one);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_one->i, one.i);
      EXPECT_EQ(vec.begin()->i, one.i);

      auto it_two = vec.insert(vec.cend(), two);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_two->i, two.i);
      EXPECT_EQ((++vec.begin())->i, two.i);

      auto it_three = vec.insert(vec.cend() - 1, three);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ(it_three->i, three.i);
      EXPECT_EQ((++vec.begin())->i, three.i);

      auto it_four = vec.insert(vec.cbegin(), four);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ(it_four->i, four.i);
      EXPECT_EQ(vec.begin()->i, four.i);

      EXPECT_EQ(vec[0].i, four.i);
      EXPECT_EQ(vec[1].i, one.i);
      EXPECT_EQ(vec[2].i, three.i);
      EXPECT_EQ(vec[3].i, two.i);
   }
}

TEST_F(tiny_dynamic_array_test, insert_rvalue_ref)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      auto it_one = vec.insert(vec.cbegin(), 1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*it_one, 1);
      EXPECT_EQ(*vec.begin(), 1);

      auto it_two = vec.insert(vec.cbegin(), 2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_two, 2);
      EXPECT_EQ(*vec.begin(), 2);
      EXPECT_EQ(*(++vec.begin()), 1);

      auto it_three = vec.insert(vec.cend() - 1, 1);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_three, 1);
      EXPECT_EQ(*(vec.end() - 1), 1);

      for (int i = 2; auto& val : vec)
      {
         EXPECT_EQ(val, i);

         if (i != 1)
         {
            --i;
         }
      }
   }

   {
      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      auto it_one = vec.insert(vec.cbegin(), copyable{1});

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_one->i, 1);
      EXPECT_EQ(vec.begin()->i, 1);

      auto it_two = vec.insert(vec.cend(), copyable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_two->i, 2);
      EXPECT_EQ((++vec.begin())->i, 2);

      auto it_three = vec.insert(vec.cend() - 1, copyable{3});

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ(it_three->i, 3);
      EXPECT_EQ((++vec.begin())->i, 3);

      auto it_four = vec.insert(vec.cbegin(), copyable{4});

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ(it_four->i, 4);
      EXPECT_EQ(vec.begin()->i, 4);

      EXPECT_EQ(vec[0].i, 4);
      EXPECT_EQ(vec[1].i, 1);
      EXPECT_EQ(vec[2].i, 3);
      EXPECT_EQ(vec[3].i, 2);
   }

   {
      core::tiny_dynamic_array<moveable, 4> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 4);

      auto it_one = vec.insert(vec.cbegin(), moveable{1});

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 4);
      EXPECT_EQ(it_one->i, 1);
      EXPECT_EQ(vec.begin()->i, 1);

      auto it_two = vec.insert(vec.cend(), moveable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 4);
      EXPECT_EQ(it_two->i, 2);
      EXPECT_EQ((++vec.begin())->i, 2);

      auto it_three = vec.insert(vec.cend() - 1, moveable{3});

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 4);
      EXPECT_EQ(it_three->i, 3);
      EXPECT_EQ((++vec.begin())->i, 3);

      auto it_four = vec.insert(vec.cbegin(), moveable{4});

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 4);
      EXPECT_EQ(it_four->i, 4);
      EXPECT_EQ(vec.begin()->i, 4);

      EXPECT_EQ(vec[0].i, 4);
      EXPECT_EQ(vec[1].i, 1);
      EXPECT_EQ(vec[2].i, 3);
      EXPECT_EQ(vec[3].i, 2);
   }
}

TEST_F(tiny_dynamic_array_test, insert_n_values)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      auto it_one = vec.insert(vec.cbegin(), 1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*it_one, 1);
      EXPECT_EQ(*vec.begin(), 1);

      auto it_two = vec.insert(vec.cend(), 2, 2);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_two, 2);

      EXPECT_EQ(vec[0], 1);
      EXPECT_EQ(vec[1], 2);
      EXPECT_EQ(vec[2], 2);

      auto it_three = vec.insert(vec.cbegin(), 4, 3);

      EXPECT_EQ(vec.size(), 7);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*it_three, 3);

      for (int i = 0; i < 4; ++i)
      {
         EXPECT_EQ(vec[i], 3);
      }
   }
}

TEST_F(tiny_dynamic_array_test, insert_range)
{
   core::tiny_dynamic_array<int, 0> vec{};

   EXPECT_EQ(vec.size(), 0);
   EXPECT_EQ(vec.capacity(), 0);

   auto it_one = vec.insert(vec.cbegin(), 1);

   EXPECT_EQ(vec.size(), 1);
   EXPECT_EQ(vec.capacity(), 1);
   EXPECT_EQ(*it_one, 1);
   EXPECT_EQ(*vec.begin(), 1);

   auto it_two = vec.insert(vec.cend(), 2, 2);

   EXPECT_EQ(vec.size(), 3);
   EXPECT_EQ(vec.capacity(), 3);
   EXPECT_EQ(*it_two, 2);

   EXPECT_EQ(vec[0], 1);
   EXPECT_EQ(vec[1], 2);
   EXPECT_EQ(vec[2], 2);

   auto it_three = vec.insert(vec.cbegin(), 4, 3);

   EXPECT_EQ(vec.size(), 7);
   EXPECT_EQ(vec.capacity(), 7);
   EXPECT_EQ(*it_three, 3);

   for (int i = 0; i < 4; ++i)
   {
      EXPECT_EQ(vec[i], 3);
   }

   {
      core::tiny_dynamic_array<int, 2> vec_range{};

      EXPECT_EQ(vec_range.size(), 0);
      EXPECT_EQ(vec_range.capacity(), 2);

      vec_range.insert(vec_range.cbegin(), vec.begin(), vec.begin() + 1);

      EXPECT_EQ(vec_range.size(), 1);
      EXPECT_EQ(vec_range.capacity(), 2);

      for (auto& val : vec_range)
      {
         EXPECT_EQ(val, 3);
      }
   }
   {
      core::tiny_dynamic_array<int, 0> vec_range{};

      EXPECT_EQ(vec_range.size(), 0);
      EXPECT_EQ(vec_range.capacity(), 0);

      vec_range.insert(vec_range.cbegin(), vec.begin(), vec.end());

      EXPECT_EQ(vec_range.size(), 7);
      EXPECT_EQ(vec_range.capacity(), 7);

      for (int i = 0; i < 4; ++i)
      {
         EXPECT_EQ(vec[i], 3);
      }
   }
}

TEST_F(tiny_dynamic_array_test, insert_initializer_list)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.insert(vec.cbegin(), {1, 2, 3});

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);

      for (int i = 1; auto& val : vec)
      {
         EXPECT_EQ(val, i++);
      }

      vec.insert(vec.cbegin(), {0, 0, 0, 0, 0});

      EXPECT_EQ(vec.size(), 8);
      EXPECT_EQ(vec.capacity(), 8);

      for (int i = 0; auto& val : vec)
      {
         if (i < 5)
         {
            EXPECT_EQ(val, 0);
         }
         else
         {
            EXPECT_EQ(val, i - 4);
         }

         ++i;
      }
   }

   {
      core::tiny_dynamic_array<copyable, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.insert(vec.cbegin(), {copyable{1}, copyable{2}, copyable{3}});

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);

      for (int i = 1; auto& val : vec)
      {
         EXPECT_EQ(val.i, i++);
      }
   }
}

TEST_F(tiny_dynamic_array_test, emplace)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      auto it_one = vec.emplace(vec.cbegin(), 1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*it_one, 1);
      EXPECT_EQ(*vec.begin(), 1);

      auto it_two = vec.emplace(vec.cbegin(), 2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_two, 2);
      EXPECT_EQ(*vec.begin(), 2);
      EXPECT_EQ(*(++vec.begin()), 1);

      auto it_three = vec.emplace(vec.cend() - 1, 1);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_three, 1);
      EXPECT_EQ(*(vec.end() - 1), 1);

      for (int i = 2; auto& val : vec)
      {
         EXPECT_EQ(val, i);

         if (i != 1)
         {
            --i;
         }
      }
   }

   {
      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      auto it_one = vec.emplace(vec.cbegin(), 1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_one->i, 1);
      EXPECT_EQ(vec.begin()->i, 1);

      auto it_two = vec.emplace(vec.cbegin(), 2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_two->i, 2);
      EXPECT_EQ(vec.begin()->i, 2);

      auto it_three = vec.emplace(vec.cend() - 1, 1);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ(it_three->i, 1);
      EXPECT_EQ((vec.end() - 1)->i, 1);

      for (int i = 2; auto& val : vec)
      {
         EXPECT_EQ(val.i, i);

         if (i != 1)
         {
            --i;
         }
      }
   }
}

TEST_F(tiny_dynamic_array_test, erase)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.push_back(1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*vec.begin(), 1);

      vec.push_back(2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(++vec.begin()), 2);

      vec.push_back(3);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(--vec.end()), 3);

      vec.push_back(4);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*(--vec.end()), 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }

      auto res_one = vec.erase(vec.cbegin());

      EXPECT_EQ(*res_one, 2);
      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 7);

      for (int i = 1; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }

      auto res_end = vec.erase(vec.cend());

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 7);

      for (int i = 1; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }

      auto res_four = vec.erase(vec.cend() - 1);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 7);

      for (int i = 1; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }
   }
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.push_back(1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*vec.begin(), 1);

      vec.push_back(2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(++vec.begin()), 2);

      vec.push_back(3);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(--vec.end()), 3);

      vec.push_back(4);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*(--vec.end()), 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }

      auto it_two = vec.erase(vec.cbegin() + 1);

      EXPECT_EQ(*it_two, 3);
      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 7);

      EXPECT_EQ(vec[0], 1);
      EXPECT_EQ(vec[1], 3);
      EXPECT_EQ(vec[2], 4);
   }
}

TEST_F(tiny_dynamic_array_test, erase_range)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.push_back(1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*vec.begin(), 1);

      vec.push_back(2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(++vec.begin()), 2);

      vec.push_back(3);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(--vec.end()), 3);

      vec.push_back(4);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*(--vec.end()), 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }

      auto it_range = vec.erase(vec.cbegin(), vec.cbegin() + 2);

      EXPECT_EQ(*it_range, 3);
      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 7);

      for (int i = 2; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }
   }
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.push_back(1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*vec.begin(), 1);

      vec.push_back(2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(++vec.begin()), 2);

      vec.push_back(3);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(--vec.end()), 3);

      vec.push_back(4);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*(--vec.end()), 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }

      auto it = vec.erase(vec.cend(), vec.cend());
      decltype(vec)::iterator end = vec.end();

      EXPECT_EQ(it, end);
   }
}

TEST_F(tiny_dynamic_array_test, push_back_lvalue_ref)
{
   {
      int one = 1;
      int two = 2;
      int three = 3;
      int four = 4;

      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.push_back(one);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*vec.begin(), one);

      vec.push_back(two);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(++vec.begin()), two);

      vec.push_back(three);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(--vec.end()), three);

      vec.push_back(four);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*(--vec.end()), four);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }
   }
   {
      copyable one{1};
      copyable two{2};
      copyable three{3};
      copyable four{4};

      core::tiny_dynamic_array<copyable, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.push_back(one);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(vec.begin()->i, one.i);

      vec.push_back(two);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ((++vec.begin())->i, two.i);

      vec.push_back(three);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ((--vec.end())->i, three.i);

      vec.push_back(four);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ((--vec.end())->i, four.i);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val.i, ++i);
      }
   }
   {
      copyable one{1};
      copyable two{2};
      copyable three{3};
      copyable four{4};

      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      vec.push_back(one);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(vec.begin()->i, one.i);

      vec.push_back(two);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ((++vec.begin())->i, two.i);

      vec.push_back(three);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ((--vec.end())->i, three.i);

      vec.push_back(four);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ((--vec.end())->i, four.i);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val.i, ++i);
      }
   }
}

TEST_F(tiny_dynamic_array_test, push_back_rvalue_ref)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.push_back(1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*vec.begin(), 1);

      vec.push_back(2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(++vec.begin()), 2);

      vec.push_back(3);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(--vec.end()), 3);

      vec.push_back(4);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*(--vec.end()), 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }
   }
   {
      core::tiny_dynamic_array<moveable, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.push_back(moveable{1});

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(vec.begin()->i, 1);

      vec.push_back(moveable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ((++vec.begin())->i, 2);

      vec.push_back(moveable{3});

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ((--vec.end())->i, 3);

      vec.push_back(moveable{4});

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ((--vec.end())->i, 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val.i, ++i);
      }
   }
   {
      core::tiny_dynamic_array<moveable, 4> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 4);

      vec.push_back(moveable{1});

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 4);
      EXPECT_EQ(vec.begin()->i, 1);

      vec.push_back(moveable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 4);
      EXPECT_EQ((++vec.begin())->i, 2);

      vec.push_back(moveable{3});

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 4);
      EXPECT_EQ((--vec.end())->i, 3);

      vec.push_back(moveable{4});

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 4);
      EXPECT_EQ((--vec.end())->i, 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val.i, ++i);
      }
   }
   {
      core::tiny_dynamic_array<moveable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      vec.push_back(moveable{1});

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(vec.begin()->i, 1);

      vec.push_back(moveable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ((++vec.begin())->i, 2);

      vec.push_back(moveable{3});

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ((--vec.end())->i, 3);

      vec.push_back(moveable{4});

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 5);
      EXPECT_EQ((--vec.end())->i, 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val.i, ++i);
      }
   }
}

TEST_F(tiny_dynamic_array_test, emplace_back)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.emplace_back(1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*vec.begin(), 1);

      vec.emplace_back(2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(++vec.begin()), 2);

      vec.emplace_back(3);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(--vec.end()), 3);

      vec.emplace_back(4);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*(--vec.end()), 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }
   }
   {
      core::tiny_dynamic_array<int*, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.emplace_back(nullptr);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*vec.begin(), nullptr);

      vec.emplace_back(nullptr);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(++vec.begin()), nullptr);

      vec.emplace_back(nullptr);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(--vec.end()), nullptr);

      vec.emplace_back(nullptr);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*(--vec.end()), nullptr);

      for (auto& val : vec)
      {
         EXPECT_EQ(val, nullptr);
      }
   }
   {
      core::tiny_dynamic_array<copyable, 3> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 3);

      vec.emplace_back(1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(vec.begin()->i, 1);

      vec.emplace_back(copyable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ((++vec.begin())->i, 2);

      vec.emplace_back(3);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ((--vec.end())->i, 3);

      vec.emplace_back(4);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ((--vec.end())->i, 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val.i, ++i);
      }
   }
   {
      core::tiny_dynamic_array<moveable, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.emplace_back(1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(vec.begin()->i, 1);

      vec.emplace_back(2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ((++vec.begin())->i, 2);

      vec.emplace_back(3);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ((--vec.end())->i, 3);

      vec.emplace_back(moveable{4});

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ((--vec.end())->i, 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val.i, ++i);
      }
   }
}

TEST_F(tiny_dynamic_array_test, pop_back)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      vec.push_back(1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*vec.begin(), 1);

      vec.push_back(2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(++vec.begin()), 2);

      vec.push_back(3);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*(--vec.end()), 3);

      vec.push_back(4);

      EXPECT_EQ(vec.size(), 4);
      EXPECT_EQ(vec.capacity(), 7);
      EXPECT_EQ(*(--vec.end()), 4);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }

      vec.pop_back();

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 7);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }

      vec.pop_back();

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 7);

      for (int i = 0; auto& val : vec)
      {
         EXPECT_EQ(val, ++i);
      }
   }
   {
      core::tiny_dynamic_array<copyable, 2> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);

      auto it_one = vec.insert(vec.cbegin(), copyable{1});

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_one->i, 1);
      EXPECT_EQ(vec.begin()->i, 1);

      auto it_two = vec.insert(vec.cend(), copyable{2});

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(it_two->i, 2);
      EXPECT_EQ((++vec.begin())->i, 2);

      vec.pop_back();

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 2);
      EXPECT_EQ(vec.begin()->i, 1);

      vec.pop_back();

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 2);
   }
}

TEST_F(tiny_dynamic_array_test, resize)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      auto it_one = vec.insert(vec.cbegin(), 1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*it_one, 1);
      EXPECT_EQ(*vec.begin(), 1);

      auto it_two = vec.insert(vec.cbegin(), 2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_two, 2);
      EXPECT_EQ(*vec.begin(), 2);
      EXPECT_EQ(*(++vec.begin()), 1);

      auto it_three = vec.insert(vec.cend() - 1, 1);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_three, 1);
      EXPECT_EQ(*(vec.end() - 1), 1);

      for (int i = 2; auto& val : vec)
      {
         EXPECT_EQ(val, i);

         if (i != 1)
         {
            --i;
         }
      }

      vec.resize(2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(vec[0], 2);
      EXPECT_EQ(vec[1], 1);
   }
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      auto it_one = vec.insert(vec.cbegin(), 1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*it_one, 1);
      EXPECT_EQ(*vec.begin(), 1);

      auto it_two = vec.insert(vec.cbegin(), 2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_two, 2);
      EXPECT_EQ(*vec.begin(), 2);
      EXPECT_EQ(*(++vec.begin()), 1);

      auto it_three = vec.insert(vec.cend() - 1, 1);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_three, 1);
      EXPECT_EQ(*(vec.end() - 1), 1);

      for (int i = 2; auto& val : vec)
      {
         EXPECT_EQ(val, i);

         if (i != 1)
         {
            --i;
         }
      }

      vec.resize(6);

      EXPECT_EQ(vec.size(), 6);
      EXPECT_EQ(vec.capacity(), 7);

      EXPECT_EQ(vec[0], 2);
      EXPECT_EQ(vec[1], 1);
      EXPECT_EQ(vec[2], 1);
      EXPECT_EQ(vec[3], 0);
      EXPECT_EQ(vec[4], 0);
      EXPECT_EQ(vec[5], 0);
   }
}

TEST_F(tiny_dynamic_array_test, resize_value)
{
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      auto it_one = vec.insert(vec.cbegin(), 1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*it_one, 1);
      EXPECT_EQ(*vec.begin(), 1);

      auto it_two = vec.insert(vec.cbegin(), 2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_two, 2);
      EXPECT_EQ(*vec.begin(), 2);
      EXPECT_EQ(*(++vec.begin()), 1);

      auto it_three = vec.insert(vec.cend() - 1, 1);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_three, 1);
      EXPECT_EQ(*(vec.end() - 1), 1);

      for (int i = 2; auto& val : vec)
      {
         EXPECT_EQ(val, i);

         if (i != 1)
         {
            --i;
         }
      }

      vec.resize(2, 0);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(vec[0], 2);
      EXPECT_EQ(vec[1], 1);
   }
   {
      core::tiny_dynamic_array<int, 0> vec{};

      EXPECT_EQ(vec.size(), 0);
      EXPECT_EQ(vec.capacity(), 0);

      auto it_one = vec.insert(vec.cbegin(), 1);

      EXPECT_EQ(vec.size(), 1);
      EXPECT_EQ(vec.capacity(), 1);
      EXPECT_EQ(*it_one, 1);
      EXPECT_EQ(*vec.begin(), 1);

      auto it_two = vec.insert(vec.cbegin(), 2);

      EXPECT_EQ(vec.size(), 2);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_two, 2);
      EXPECT_EQ(*vec.begin(), 2);
      EXPECT_EQ(*(++vec.begin()), 1);

      auto it_three = vec.insert(vec.cend() - 1, 1);

      EXPECT_EQ(vec.size(), 3);
      EXPECT_EQ(vec.capacity(), 3);
      EXPECT_EQ(*it_three, 1);
      EXPECT_EQ(*(vec.end() - 1), 1);

      for (int i = 2; auto& val : vec)
      {
         EXPECT_EQ(val, i);

         if (i != 1)
         {
            --i;
         }
      }

      vec.resize(6, 10);

      EXPECT_EQ(vec.size(), 6);
      EXPECT_EQ(vec.capacity(), 7);

      EXPECT_EQ(vec[0], 2);
      EXPECT_EQ(vec[1], 1);
      EXPECT_EQ(vec[2], 1);
      EXPECT_EQ(vec[3], 10);
      EXPECT_EQ(vec[4], 10);
      EXPECT_EQ(vec[5], 10);
   }
}
