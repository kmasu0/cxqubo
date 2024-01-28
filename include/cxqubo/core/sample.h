/// Copyright 2024 Koichi Masuda
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.

#ifndef CXQUBO_CORE_SAMPLE_H
#define CXQUBO_CORE_SAMPLE_H

#include "cxqubo/core/vartypes.h"
#include <unordered_map>

namespace cxqubo {
using Sample = std::unordered_map<unsigned, int32_t>;
using DecodedSample = std::unordered_map<std::string_view, int32_t>;
} // namespace cxqubo

#endif
