// Copyright 2015 TIER IV, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest/gtest.h>

#include <geometry/bounding_box.hpp>
#include <geometry/distance.hpp>
#include <scenario_simulator_exception/exception.hpp>

geometry_msgs::msg::Point makePoint(double x, double y, double z = 0.0)
{
  geometry_msgs::msg::Point p;
  p.x = x;
  p.y = y;
  p.z = z;
  return p;
}

geometry_msgs::msg::Pose makePose(
  double x, double y, double z = 0.0,
  geometry_msgs::msg::Quaternion q = geometry_msgs::msg::Quaternion())
{
  geometry_msgs::msg::Pose p;
  p.position.x = x;
  p.position.y = y;
  p.position.z = z;
  p.orientation = q;
  return p;
}

TEST(Distance, PointToPoint)
{
  geometry_msgs::msg::Point p00, p01;
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p00, p01), 0);

  geometry_msgs::msg::Point p10, p11 = makePoint(1, 0);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p10, p11), 1);

  geometry_msgs::msg::Point p20 = makePoint(0, 1);
  geometry_msgs::msg::Point p21 = makePoint(1, 0);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p20, p21), std::sqrt(2));
}

TEST(Distance, PoseToPose)
{
  geometry_msgs::msg::Pose p00, p01;
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p00, p01), 0);

  geometry_msgs::msg::Pose p10, p11 = makePose(1, 0);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p10, p11), 1);

  geometry_msgs::msg::Pose p20 = makePose(0, 1);
  geometry_msgs::msg::Pose p21 = makePose(1, 0);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p20, p21), std::sqrt(2));
}

TEST(Distance, PointToPose)
{
  geometry_msgs::msg::Point p00;
  geometry_msgs::msg::Pose p01;
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p00, p01), 0);

  geometry_msgs::msg::Point p10;
  geometry_msgs::msg::Pose p11 = makePose(1, 0);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p10, p11), 1);

  geometry_msgs::msg::Point p20 = makePoint(0, 1);
  geometry_msgs::msg::Pose p21 = makePose(1, 0);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p20, p21), std::sqrt(2));
}

TEST(Distance, PoseToPoint)
{
  geometry_msgs::msg::Pose p00;
  geometry_msgs::msg::Point p01;
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p00, p01), 0);

  geometry_msgs::msg::Pose p10;
  geometry_msgs::msg::Point p11 = makePoint(1, 0);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p10, p11), 1);

  geometry_msgs::msg::Pose p20 = makePose(0, 1);
  geometry_msgs::msg::Point p21 = makePoint(1, 0);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance(p20, p21), std::sqrt(2));
}

TEST(Distance, Distance2DDisjoint)
{
  std::vector<geometry_msgs::msg::Point> polygon0(4), polygon1(4);
  polygon0[0] = makePoint(0, 0);
  polygon0[1] = makePoint(0, 1);
  polygon0[2] = makePoint(1, 1);
  polygon0[3] = makePoint(1, 0);
  polygon1[0] = makePoint(2, 2);
  polygon1[1] = makePoint(2, 3);
  polygon1[2] = makePoint(3, 3);
  polygon1[3] = makePoint(3, 2);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance2D(polygon0, polygon1), std::sqrt(2));
}

TEST(Distance, Distance2DIntersect)
{
  std::vector<geometry_msgs::msg::Point> polygon0(4), polygon1(4);
  polygon0[0] = makePoint(0, 0);
  polygon0[1] = makePoint(0, 2);
  polygon0[2] = makePoint(2, 2);
  polygon0[3] = makePoint(2, 0);
  polygon1[0] = makePoint(1, 1);
  polygon1[1] = makePoint(1, 3);
  polygon1[2] = makePoint(3, 3);
  polygon1[3] = makePoint(3, 1);
  EXPECT_DOUBLE_EQ(math::geometry::getDistance2D(polygon0, polygon1), 0);
}

TEST(Distance, Distance2DZeroElements)
{
  std::vector<geometry_msgs::msg::Point> polygon0(4), polygon1;
  polygon0[0] = makePoint(0, 0);
  polygon0[1] = makePoint(0, 1);
  polygon0[2] = makePoint(1, 1);
  polygon0[3] = makePoint(1, 0);
  EXPECT_THROW(
    math::geometry::getDistance2D(polygon0, polygon1), boost::geometry::empty_input_exception);
}

int main(int argc, char ** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
