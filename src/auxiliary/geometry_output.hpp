/*
   Copyright 2016 Mitchell Young

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#include "util/pugifwd.hpp"
#include "core/core_mesh.hpp"
#include "moc/ray_data.hpp"

namespace mocc {
namespace aux {
void output_geometry(const pugi::xml_node &input, const CoreMesh &mesh);
}
} // namespaces
