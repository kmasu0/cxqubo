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

#ifndef CXQUBO_CONFIG_H
#define CXQUBO_CONFIG_H

#cmakedefine CXQUBO_ENABLE_DUMP

#define CXQUBO_VERSION_MAJOR ${CXQUBO_VERSION_MAJOR}

#define CXQUBO_VERSION_MINOR ${CXQUBO_VERSION_MINOR}

#define CXQUBO_VERSION_PATCH ${CXQUBO_VERSION_PATCH}

#define CXQUBO_VERSION_STRING "${PACKAGE_VERSION}"

#endif
