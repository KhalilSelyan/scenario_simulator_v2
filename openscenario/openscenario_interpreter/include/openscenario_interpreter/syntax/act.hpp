// Copyright 2015-2020 Tier IV, Inc. All rights reserved.
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

#ifndef OPENSCENARIO_INTERPRETER__SYNTAX__ACT_HPP_
#define OPENSCENARIO_INTERPRETER__SYNTAX__ACT_HPP_

#include <nlohmann/json.hpp>
#include <openscenario_interpreter/syntax/maneuver_group.hpp>
#include <openscenario_interpreter/syntax/storyboard_element.hpp>

namespace openscenario_interpreter
{
inline namespace syntax
{
/* ---- Act --------------------------------------------------------------------
 *
 *  <xsd:complexType name="Act">
 *    <xsd:sequence>
 *      <xsd:element name="ManeuverGroup" maxOccurs="unbounded" type="ManeuverGroup"/>
 *      <xsd:element name="StartTrigger" type="Trigger"/>
 *      <xsd:element name="StopTrigger" minOccurs="0" type="Trigger"/>
 *    </xsd:sequence>
 *    <xsd:attribute name="name" type="String" use="required"/>
 *  </xsd:complexType>
 *
 * -------------------------------------------------------------------------- */
struct Act : public Scope, public StoryboardElement<Act>, public Elements
{
  Trigger start_trigger;

  Element stop_trigger;

  template <typename Node>
  explicit Act(const Node & node, Scope & outer_scope)
  : Scope(outer_scope.makeChildScope(readAttribute<String>("name", node, outer_scope))),
    start_trigger(readElement<Trigger>("StartTrigger", node, localScope()))
  {
    callWithElements(node, "ManeuverGroup", 1, unbounded, [&](auto && node) {
      return push_back(readStoryboardElement<ManeuverGroup>(node, localScope()));
    });

    callWithElements(node, "StopTrigger", 0, 1, [&](auto && node) {
      return stop_trigger.rebind<Trigger>(node, localScope());
    });
  }

  using StoryboardElement::evaluate;

  static constexpr auto start() noexcept -> void {}

  auto accomplished() const -> bool;

  auto stop() -> void;

  auto stopTriggered() const -> bool;

  auto ready() -> bool;

  auto run() -> void;
};

auto operator<<(nlohmann::json &, const Act &) -> nlohmann::json &;
}  // namespace syntax
}  // namespace openscenario_interpreter

#endif  // OPENSCENARIO_INTERPRETER__SYNTAX__ACT_HPP_
