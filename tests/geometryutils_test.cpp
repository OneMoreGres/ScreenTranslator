#include <gtest/gtest.h>

#include "geometryutils.h"

#include <QDebug>
#include <QRect>

using namespace service::geometry;

TEST(GeometryUtils, CornerAtPoint)
{
  // outside
  {
    const auto testee = cornerAtPoint({0, 0}, {10, 10}, {10, 10, 50, 50});
    EXPECT_EQ(QPoint(10, 10), testee);
  }
  {
    const auto testee = cornerAtPoint({200, 200}, {10, 10}, {10, 10, 50, 50});
    EXPECT_EQ(QPoint(50, 50), testee);
  }
  {
    const auto testee = cornerAtPoint({200, 20}, {10, 10}, {10, 10, 50, 50});
    EXPECT_EQ(QPoint(50, 20), testee);
  }

  // top left
  {
    const auto testee = cornerAtPoint({10, 10}, {10, 10}, {0, 0, 100, 100});
    EXPECT_EQ(QPoint(10, 10), testee);
  }
  // top right
  {
    const auto testee = cornerAtPoint({95, 10}, {10, 10}, {0, 0, 100, 100});
    EXPECT_EQ(QPoint(86, 10), testee);
  }
  // bottom left
  {
    const auto testee = cornerAtPoint({10, 95}, {10, 10}, {0, 0, 100, 100});
    EXPECT_EQ(QPoint(10, 86), testee);
  }
  // bottom right
  {
    const auto testee = cornerAtPoint({95, 95}, {10, 10}, {0, 0, 100, 100});
    EXPECT_EQ(QPoint(86, 86), testee);
  }
}
