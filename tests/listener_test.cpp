#include <cstddef>
#include "../listener.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using testing::StrictMock;

class TileMock : public Tile {
 public:
  MOCK_METHOD0(render, void());
};

class ListenerTest : public ::testing::Test {
 protected:
  ListenerTest() {
  }

  ~ListenerTest() override {
  }
  
  StrictMock<TileMock> root_;
  StrictMock<TileMock> child1_;
  StrictMock<TileMock> child2_;
  StrictMock<TileMock> child3_;
  StrictMock<TileMock> child1_child1_;
  StrictMock<TileMock> child1_child2_;
  StrictMock<TileMock> child2_child1_;
};

// Tests that the Foo::Bar() method does Abc.
TEST_F(ListenerTest, ActiveAdd) {
  root_.add_child(&child1_, true);
  EXPECT_EQ(root_.active_, &child1_);
  root_.add_child(&child2_, true);
  EXPECT_EQ(root_.active_, &child2_);
  EXPECT_EQ(child2_.next_, &child1_);
  EXPECT_EQ(NULL, child1_.next_);
  EXPECT_EQ(NULL, child2_.prev_);
  EXPECT_EQ(child1_.prev_, &child2_);
  EXPECT_EQ(NULL, root_.standby_);
}

TEST_F(ListenerTest, UnlinkHead) {
  root_.add_child(&child2_, true);
  root_.add_child(&child1_, true);
  child1_.unlink(&(root_.active_));
  EXPECT_EQ(root_.active_, &child2_);
  EXPECT_EQ(NULL, child1_.next_);
  EXPECT_EQ(NULL, child1_.prev_);
  EXPECT_EQ(NULL, child2_.next_);
  EXPECT_EQ(NULL, child2_.prev_);
}

TEST_F(ListenerTest, UnlinkTail) {
  root_.add_child(&child2_, true);
  root_.add_child(&child1_, true);
  child2_.unlink(&(root_.active_));
  EXPECT_EQ(root_.active_, &child1_);
  EXPECT_EQ(NULL, child1_.next_);
  EXPECT_EQ(NULL, child1_.prev_);
  EXPECT_EQ(NULL, child2_.next_);
  EXPECT_EQ(NULL, child2_.prev_);
}

TEST_F(ListenerTest, ExplicitSwapHead) {
  root_.add_child(&child2_, true);
  root_.add_child(&child1_, true);
  root_.deactivate_child(&child1_);
  EXPECT_EQ(root_.standby_, &child1_);
  EXPECT_EQ(root_.active_, &child2_);
  EXPECT_EQ(NULL, child1_.next_);
  EXPECT_EQ(NULL, child2_.next_);
  EXPECT_EQ(NULL, child1_.prev_);
  EXPECT_EQ(NULL, child2_.prev_);
}

TEST_F(ListenerTest, ExplicitActivate) {
  root_.add_child(&child2_, false);
  root_.add_child(&child1_, false);
  root_.activate_child(&child2_);
  EXPECT_EQ(root_.standby_, &child1_);
  EXPECT_EQ(root_.active_, &child2_);
  EXPECT_EQ(NULL, child1_.next_);
  EXPECT_EQ(NULL, child2_.next_);
  EXPECT_EQ(NULL, child1_.prev_);
  EXPECT_EQ(NULL, child2_.prev_);
}

TEST_F(ListenerTest, ExplicitSwapTail) {
  root_.add_child(&child2_, true);
  root_.add_child(&child1_, true);
  root_.deactivate_child(&child2_);
  EXPECT_EQ(root_.standby_, &child2_);
  EXPECT_EQ(root_.active_, &child1_);
  EXPECT_EQ(NULL, child1_.next_);
  EXPECT_EQ(NULL, child2_.next_);
  EXPECT_EQ(NULL, child1_.prev_);
  EXPECT_EQ(NULL, child2_.prev_);
}

TEST_F(ListenerTest, ExplicitSwapMid) {
  root_.add_child(&child3_, true);
  root_.add_child(&child2_, true);
  root_.add_child(&child1_, true);
  root_.deactivate_child(&child2_);
  EXPECT_EQ(root_.standby_, &child2_);
  EXPECT_EQ(root_.active_, &child1_);
  EXPECT_EQ(&child3_, child1_.next_);
  EXPECT_EQ(&child1_, child3_.prev_);
  EXPECT_EQ(NULL, child3_.next_);
  EXPECT_EQ(NULL, child1_.prev_);
  EXPECT_EQ(NULL, child2_.next_);
  EXPECT_EQ(NULL, child2_.prev_);
}

TEST_F(ListenerTest, ToggleHead) {
  root_.add_child(&child2_, false);
  root_.add_child(&child1_, false);
  root_.toggle_child(&child1_);
  EXPECT_EQ(root_.standby_, &child2_);
  EXPECT_EQ(root_.active_, &child1_);
  EXPECT_EQ(NULL, child1_.next_);
  EXPECT_EQ(NULL, child2_.next_);
  EXPECT_EQ(NULL, child1_.prev_);
  EXPECT_EQ(NULL, child2_.prev_);
}

TEST_F(ListenerTest, ToggleTail) {
  root_.add_child(&child2_, false);
  root_.add_child(&child1_, false);
  root_.toggle_child(&child2_);
  EXPECT_EQ(root_.standby_, &child1_);
  EXPECT_EQ(root_.active_, &child2_);
  EXPECT_EQ(NULL, child1_.next_);
  EXPECT_EQ(NULL, child2_.next_);
  EXPECT_EQ(NULL, child1_.prev_);
  EXPECT_EQ(NULL, child2_.prev_);
}

TEST_F(ListenerTest, ToggleMid) {
  root_.add_child(&child3_, false);
  root_.add_child(&child2_, false);
  root_.add_child(&child1_, false);
  root_.toggle_child(&child2_);
  EXPECT_EQ(root_.active_, &child2_);
  EXPECT_EQ(root_.standby_, &child1_);
  EXPECT_EQ(&child3_, child1_.next_);
  EXPECT_EQ(&child1_, child3_.prev_);
  EXPECT_EQ(NULL, child3_.next_);
  EXPECT_EQ(NULL, child1_.prev_);
  EXPECT_EQ(NULL, child2_.next_);
  EXPECT_EQ(NULL, child2_.prev_);
}

TEST_F(ListenerTest, RenderEventPropagate) {
  root_.add_child(&child3_, true);
  root_.add_child(&child2_, false);
  root_.add_child(&child1_, true);
  child1_.add_child(&child1_child2_, false);
  child1_.add_child(&child1_child1_, true);
  child2_.add_child(&child2_child1_, true);
  
  EXPECT_CALL(root_, render()).Times(1);
  EXPECT_CALL(child1_, render()).Times(1);
  EXPECT_CALL(child3_, render()).Times(1);
  EXPECT_CALL(child1_child1_, render()).Times(1);

  root_.on_event(EVENT_RENDER);
} 
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
