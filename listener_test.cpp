#include <cstddef>
#include "../listener.h"
#include "gtest/gtest.h"


class ListenerTest : public ::testing::Test {
 protected:
  ListenerTest() {
  }

  ~ListenerTest() override {
  }
  
  Listener root_;
  Listener child1_;
};

// Tests that the Foo::Bar() method does Abc.
TEST_F(ListenerTest, ActiveAdd) {
  root_.add_child(&child1_, true);
}


