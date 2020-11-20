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

#include <awapi_awauto_adapter/autoware_auto_adapter.hpp>
#include <rclcpp_components/register_node_macro.hpp>

namespace autoware_api
{

AutowareAutoAdapter::AutowareAutoAdapter(const rclcpp::NodeOptions & options)
: rclcpp::Node("autoware_auto_adapter", options)
{}

}  // namespace autoware_api

RCLCPP_COMPONENTS_REGISTER_NODE(autoware_api::AutowareAutoAdapter)