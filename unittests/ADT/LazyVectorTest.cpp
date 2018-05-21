//===- llvm/unittest/ADT/LazyVectorTest.cpp - PackedVector tests --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/LazyVector.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {

struct TestElement {
  int Data = 1234;
  TestElement() {}
  TestElement(int Data) : Data(Data) {}

  bool isDefaultInit() { return Data == 1234; }
};

TEST(LazyVectorTest, Init) {
  LazyVector<TestElement, 1024> Vec;
  EXPECT_EQ(0U, Vec.size());
  EXPECT_EQ(0U, Vec.capacity());
  EXPECT_TRUE(Vec.empty());
}

TEST(LazyVectorTest, Operation) {
  LazyVector<TestElement, 1024> Vec;

  Vec.resize(5);
  EXPECT_EQ(5U, Vec.size());
  EXPECT_FALSE(Vec.empty());

  Vec[1] = 4;

  EXPECT_EQ(5U, Vec.size());
  EXPECT_EQ(4, Vec[1].Data);
  EXPECT_FALSE(Vec.empty());

  Vec.resize(2000);
  EXPECT_EQ(2000U, Vec.size());
  EXPECT_EQ(4, Vec[1].Data);
  EXPECT_FALSE(Vec.empty());

  Vec.resize(0U);
  EXPECT_EQ(0U, Vec.size());
  EXPECT_TRUE(Vec.empty());
}

// Tests that default initializers are called.
TEST(LazyVectorTest, DefaultInit) {
  LazyVector<TestElement, 1024> Vec;
  Vec.resize(1000);
  ASSERT_TRUE(Vec[20].isDefaultInit());

  Vec.resize(3000);
  ASSERT_TRUE(Vec[20].isDefaultInit());
  ASSERT_TRUE(Vec[2000].isDefaultInit());
}

// Tests that we allocate on demand.
TEST(LazyVectorTest, Lazyness) {
  LazyVector<TestElement, 100> Vec;
  EXPECT_EQ(0U, Vec.allocatedElements());

  Vec.resize(3);
  EXPECT_EQ(0U, Vec.allocatedElements());

  Vec[1] = 1;
  EXPECT_EQ(100U, Vec.allocatedElements());

  Vec.resize(3000);
  Vec[500] = 1;
  // Two chunks (each 100 elements) should have been allocated.
  EXPECT_EQ(200U, Vec.allocatedElements());

  Vec.resize(300);
  EXPECT_EQ(100U, Vec.allocatedElements());

  Vec.resize(5);
  EXPECT_EQ(100U, Vec.allocatedElements());

  Vec.resize(0);
  EXPECT_EQ(0U, Vec.allocatedElements());
}

// Tests that growing the vector doesn't corrupt any data.
TEST(LazyVectorTest, Grow) {
  LazyVector<TestElement, 1024> Vec;

  Vec.resize(100);
  Vec[1] = 4;

  EXPECT_EQ(100U, Vec.size());
  EXPECT_EQ(4, Vec[1].Data);
  EXPECT_FALSE(Vec.empty());

  Vec.resize(1500);
  Vec[1400] = 44;

  EXPECT_EQ(1500U, Vec.size());
  EXPECT_EQ(4, Vec[1].Data);
  EXPECT_EQ(44, Vec[1400].Data);
  EXPECT_FALSE(Vec.empty());

  Vec.resize(8500);
  Vec[4400] = 444;

  EXPECT_EQ(8500U, Vec.size());
  EXPECT_EQ(4, Vec[1].Data);
  EXPECT_EQ(44, Vec[1400].Data);
  EXPECT_EQ(444, Vec[4400].Data);
  EXPECT_FALSE(Vec.empty());
}

// Tests that shrinking the vector doesn't corrupt any data.
TEST(LazyVectorTest, Shrink) {
  LazyVector<TestElement, 1024> Vec;

  Vec.resize(2000);
  Vec[1] = 4;
  Vec[1400] = 44;

  Vec.resize(1500);
  EXPECT_EQ(1500U, Vec.size());
  EXPECT_EQ(44, Vec[1400].Data);
  EXPECT_EQ(4, Vec[1].Data);
  EXPECT_FALSE(Vec.empty());

  Vec.resize(1000);
  EXPECT_EQ(1000U, Vec.size());
  EXPECT_EQ(4, Vec[1].Data);
  EXPECT_FALSE(Vec.empty());

  Vec.resize(300);
  EXPECT_EQ(300U, Vec.size());
  EXPECT_EQ(4, Vec[1].Data);
  EXPECT_FALSE(Vec.empty());

  Vec.resize(0);
  EXPECT_EQ(0U, Vec.size());
  EXPECT_TRUE(Vec.empty());
}

// Tests that the vector can grow again after shrinking.
TEST(LazyVectorTest, ShrinkAndGrow) {
  LazyVector<TestElement, 1024> Vec;

  Vec.resize(2000);
  Vec[1] = 4;
  Vec[1400] = 44;

  Vec.resize(0);

  Vec.resize(2000);
  Vec[1] = 4;
  Vec[1400] = 44;

  EXPECT_EQ(4, Vec[1].Data);
  EXPECT_EQ(44, Vec[1400].Data);
  EXPECT_EQ(2000U, Vec.size());
  EXPECT_FALSE(Vec.empty());
}
} // namespace
