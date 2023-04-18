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

#include <openscenario_interpreter/object.hpp>
#include <openscenario_interpreter/reader/element.hpp>
#include <openscenario_interpreter/scope.hpp>
#include <openscenario_interpreter/syntax/by_type.hpp>
#include <openscenario_interpreter/syntax/entity_ref.hpp>
#include <openscenario_interpreter/syntax/selected_entities.hpp>
#include <utility>

namespace openscenario_interpreter
{
inline namespace syntax
{
SelectedEntityRefs::SelectedEntityRefs(const pugi::xml_node & tree, Scope & scope)
: entityRefs(readElements<EntityRef, 0>("EntityRef", tree, scope))
{
}

SelectedByTypes::SelectedByTypes(const pugi::xml_node & tree, Scope & scope)
: byTypes(readElements<ByType, 0>("ByType", tree, scope))
{
}

SelectedEntities::SelectedEntities(const pugi::xml_node & tree, Scope & scope)
// clang-format off
: ComplexType(
    choice(tree,
      std::make_pair("EntityRef", [&](const auto & tree) { return make<SelectedEntityRefs>(tree, scope); }),
      std::make_pair(   "ByType", [&](const auto & tree) { return make<   SelectedByTypes>(tree, scope); })))
// clang-format on
{
}
}  // namespace syntax
}  // namespace openscenario_interpreter
