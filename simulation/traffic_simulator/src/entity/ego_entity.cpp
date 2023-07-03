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

#include <quaternion_operation/quaternion_operation.h>

#include <boost/lexical_cast.hpp>
#include <concealer/autoware_universe.hpp>
#include <concealer/field_operator_application_for_autoware_universe.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <thread>
#include <traffic_simulator/entity/ego_entity.hpp>
#include <traffic_simulator_msgs/msg/waypoints_array.hpp>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace traffic_simulator
{
namespace entity
{
/// @todo find some shared space for this function
template <typename T>
static auto getParameter(const std::string & name, T value = {})
{
  rclcpp::Node node{"get_parameter", "simulation"};

  node.declare_parameter<T>(name, value);
  node.get_parameter<T>(name, value);

  return value;
}

auto EgoEntity::makeFieldOperatorApplication(const Configuration & configuration)
  -> std::unique_ptr<concealer::FieldOperatorApplication>
{
  if (const auto architecture_type = getParameter<std::string>("architecture_type", "awf/universe");
      architecture_type == "awf/universe") {
    std::string rviz_config = getParameter<std::string>("rviz_config", "");
    return getParameter<bool>("launch_autoware", true)
             ? std::make_unique<
                 concealer::FieldOperatorApplicationFor<concealer::AutowareUniverse>>(
                 getParameter<std::string>("autoware_launch_package"),
                 getParameter<std::string>("autoware_launch_file"),
                 "map_path:=" + configuration.map_path.string(),
                 "lanelet2_map_file:=" + configuration.getLanelet2MapFile(),
                 "pointcloud_map_file:=" + configuration.getPointCloudMapFile(),
                 "sensor_model:=" + getParameter<std::string>("sensor_model"),
                 "vehicle_model:=" + getParameter<std::string>("vehicle_model"),
                 "rviz_config:=" + ((rviz_config == "")
                                      ? configuration.rviz_config_path.string()
                                      : Configuration::Pathname(rviz_config).string()),
                 "scenario_simulation:=true", "perception/enable_traffic_light:=false")
             : std::make_unique<
                 concealer::FieldOperatorApplicationFor<concealer::AutowareUniverse>>();
  } else {
    throw common::SemanticError(
      "Unexpected architecture_type ", std::quoted(architecture_type), " was given.");
  }
}

EgoEntity::EgoEntity(
  const std::string & name, const traffic_simulator_msgs::msg::EntityStatus & entity_status,
  const traffic_simulator_msgs::msg::VehicleParameters & parameters,
  const Configuration & configuration)
: VehicleEntity(name, entity_status, parameters),
  field_operator_application(makeFieldOperatorApplication(configuration))
{
}

auto EgoEntity::asFieldOperatorApplication() const -> concealer::FieldOperatorApplication &
{
  assert(field_operator_application);
  return *field_operator_application;
}

auto EgoEntity::getCurrentAction() const -> std::string
{
  const auto state = field_operator_application->getAutowareStateName();
  return state.empty() ? "Launching" : state;
}

auto EgoEntity::getBehaviorParameter() const -> traffic_simulator_msgs::msg::BehaviorParameter
{
  traffic_simulator_msgs::msg::BehaviorParameter parameter;
  /**
   * @brief TODO, Input values get from autoware.
   */
  parameter.see_around = true;
  parameter.dynamic_constraints.max_acceleration = 0;
  parameter.dynamic_constraints.max_deceleration = 0;
  return parameter;
}

auto EgoEntity::getEntityTypename() const -> const std::string &
{
  static const std::string result = "EgoEntity";
  return result;
}

auto EgoEntity::getObstacle() -> std::optional<traffic_simulator_msgs::msg::Obstacle>
{
  return std::nullopt;
}

auto EgoEntity::getRouteLanelets(double /*unused horizon*/) const -> std::vector<std::int64_t>
{
  std::vector<std::int64_t> ids{};

  if (const auto universe =
        dynamic_cast<concealer::FieldOperatorApplicationFor<concealer::AutowareUniverse> *>(
          field_operator_application.get());
      universe) {
    for (const auto & point : universe->getPathWithLaneId().points) {
      std::copy(point.lane_ids.begin(), point.lane_ids.end(), std::back_inserter(ids));
    }
  }

  return ids;
}

auto EgoEntity::getCurrentPose() const -> geometry_msgs::msg::Pose { return status_.pose; }

auto EgoEntity::getWaypoints() -> const traffic_simulator_msgs::msg::WaypointsArray
{
  return field_operator_application->getWaypoints();
}

void EgoEntity::onUpdate(double current_time, double step_time)
{
  EntityBase::onUpdate(current_time, step_time);
  setStatus(externally_updated_status_);

  updateStandStillDuration(step_time);
  updateTraveledDistance(step_time);

  field_operator_application->rethrow();
  field_operator_application->spinSome();

  EntityBase::onPostUpdate(current_time, step_time);
}

void EgoEntity::requestAcquirePosition(
  const traffic_simulator_msgs::msg::LaneletPose & lanelet_pose)
{
  requestAssignRoute({lanelet_pose});
}

void EgoEntity::requestAcquirePosition(const geometry_msgs::msg::Pose & map_pose)
{
  requestAssignRoute({map_pose});
}

void EgoEntity::requestAssignRoute(
  const std::vector<traffic_simulator_msgs::msg::LaneletPose> & waypoints)
{
  std::vector<geometry_msgs::msg::Pose> route;

  for (const auto & waypoint : waypoints) {
    route.push_back((*hdmap_utils_ptr_).toMapPose(waypoint).pose);
  }

  requestAssignRoute(route);
}

void EgoEntity::requestAssignRoute(const std::vector<geometry_msgs::msg::Pose> & waypoints)
{
  std::vector<geometry_msgs::msg::PoseStamped> route;

  for (const auto & waypoint : waypoints) {
    geometry_msgs::msg::PoseStamped pose_stamped;
    {
      pose_stamped.header.frame_id = "map";
      pose_stamped.pose = waypoint;
    }

    route.push_back(pose_stamped);
  }

  if (not field_operator_application->initialized()) {
    field_operator_application->initialize(getStatus().pose);
    field_operator_application->plan(route);
    // NOTE: engage() will be executed at simulation-time 0.
  } else {
    field_operator_application->plan(route);
    field_operator_application->engage();
  }
}

void EgoEntity::requestLaneChange(const std::int64_t)
{
  THROW_SEMANTIC_ERROR(
    "From scenario, a lane change was requested to Ego type entity ", std::quoted(name),
    " In general, such a request is an error, since Ego cars make autonomous decisions about "
    "everything but their destination");
}

auto EgoEntity::requestLaneChange(const traffic_simulator::lane_change::Parameter &) -> void
{
  THROW_SEMANTIC_ERROR(
    "From scenario, a lane change was requested to Ego type entity ", std::quoted(name),
    " In general, such a request is an error, since Ego cars make autonomous decisions about "
    "everything but their destination");
}

auto EgoEntity::requestSpeedChange(
  const double target_speed, const speed_change::Transition, const speed_change::Constraint,
  const bool) -> void
{
  requestSpeedChange(target_speed, false);
}

auto EgoEntity::requestSpeedChange(
  const speed_change::RelativeTargetSpeed &, const speed_change::Transition,
  const speed_change::Constraint, const bool) -> void
{
  THROW_SEMANTIC_ERROR(
    "The traffic_simulator's request to set speed to the Ego type entity is for initialization "
    "purposes only.");
}

auto EgoEntity::getDefaultDynamicConstraints() const
  -> const traffic_simulator_msgs::msg::DynamicConstraints &
{
  THROW_SEMANTIC_ERROR("getDefaultDynamicConstraints function does not support EgoEntity");
}

auto EgoEntity::setBehaviorParameter(const traffic_simulator_msgs::msg::BehaviorParameter &) -> void
{
}

auto EgoEntity::setStatusExternally(const traffic_simulator_msgs::msg::EntityStatus & status)
  -> void
{
  externally_updated_status_ = status;
}

void EgoEntity::requestSpeedChange(double value, bool)
{
  field_operator_application->restrictTargetSpeed(value);
}

void EgoEntity::requestSpeedChange(
  const speed_change::RelativeTargetSpeed & /*target_speed*/, bool /*continuous*/)
{
}

auto EgoEntity::setVelocityLimit(double value) -> void  //
{
  field_operator_application->setVelocityLimit(value);
}

auto EgoEntity::fillLaneletPose(traffic_simulator_msgs::msg::EntityStatus & status) const -> void
{
  const auto unique_route_lanelets = traffic_simulator::helper::getUniqueValues(getRouteLanelets());

  std::optional<traffic_simulator_msgs::msg::LaneletPose> lanelet_pose;

  if (unique_route_lanelets.empty()) {
    lanelet_pose =
      hdmap_utils_ptr_->toLaneletPose(status.pose, getStatus().bounding_box, false, 1.0);
  } else {
    lanelet_pose = hdmap_utils_ptr_->toLaneletPose(status.pose, unique_route_lanelets, 1.0);
    if (!lanelet_pose) {
      lanelet_pose =
        hdmap_utils_ptr_->toLaneletPose(status.pose, getStatus().bounding_box, false, 1.0);
    }
  }
  if (lanelet_pose) {
    math::geometry::CatmullRomSpline spline(
      hdmap_utils_ptr_->getCenterPoints(lanelet_pose->lanelet_id));
    if (const auto s_value = spline.getSValue(status.pose)) {
      status.pose.position.z = spline.getPoint(s_value.value()).z;
    }
  }

  status.lanelet_pose_valid = static_cast<bool>(lanelet_pose);
  if (status.lanelet_pose_valid) {
    status.lanelet_pose = lanelet_pose.value();
  }
}

}  // namespace entity
}  // namespace traffic_simulator
