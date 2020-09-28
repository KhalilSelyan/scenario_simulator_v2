// Copyright 2015-2020 Autoware Foundation. All rights reserved.
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

#ifndef SCENARIO_RUNNER__SYNTAX__LATERAL_ACTION_HPP_
#define SCENARIO_RUNNER__SYNTAX__LATERAL_ACTION_HPP_

#include <scenario_runner/syntax/lane_change_action.hpp>

namespace scenario_runner
{
inline namespace syntax
{
/* ==== LateralAction ========================================================
 *
 * <xsd:complexType name="LateralAction">
 *   <xsd:choice>
 *     <xsd:element name="LaneChangeAction" type="LaneChangeAction"/>
 *     <xsd:element name="LaneOffsetAction" type="LaneOffsetAction"/>
 *     <xsd:element name="LateralDistanceAction" type="LateralDistanceAction"/>
 *   </xsd:choice>
 * </xsd:complexType>
 *
 * ======================================================================== */
struct LateralAction
  : public Object
{
  template<typename Node, typename Scope>
  explicit LateralAction(const Node & node, Scope & scope)
  {
    callWithElements(node, "LaneChangeAction", 0, 1, [&](auto && node)
      {
        return rebind<LaneChangeAction>(node, scope);
      });

    callWithElements(node, "LaneOffsetAction", 0, 1, THROW_UNSUPPORTED_ERROR(node));
    callWithElements(node, "LateralDistanceAction", 0, 1, THROW_UNSUPPORTED_ERROR(node));
  }
};
}
}  // namespace scenario_runner

#endif  // SCENARIO_RUNNER__SYNTAX__LATERAL_ACTION_HPP_