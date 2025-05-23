# Copyright 2010-2025 Google LLC
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set_property(SOURCE graph.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE graph.i PROPERTY SWIG_MODULE_NAME operations_research_graph)
set_property(SOURCE graph.i PROPERTY COMPILE_DEFINITIONS
  ${OR_TOOLS_COMPILE_DEFINITIONS} ABSL_MUST_USE_RESULT=)
set_property(SOURCE graph.i PROPERTY COMPILE_OPTIONS
  -namespace ${DOTNET_PROJECT}.Graph
  -dllimport google-ortools-native)
swig_add_library(dotnet_graph
  TYPE OBJECT
  LANGUAGE csharp
  OUTPUT_DIR ${DOTNET_PROJECT_DIR}/ortools/graph
  SOURCES graph.i)

#target_include_directories(dotnet_graph PRIVATE ${DOTNET_INCLUDE_DIRS})
set_target_properties(dotnet_graph PROPERTIES
  SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON
  POSITION_INDEPENDENT_CODE ON)
target_link_libraries(dotnet_graph PRIVATE ortools::ortools)

if(BUILD_TESTING)
  file(GLOB DOTNET_SRCS "*Tests.cs")
  foreach(FILE_NAME IN LISTS DOTNET_SRCS)
    add_dotnet_test(FILE_NAME ${FILE_NAME})
  endforeach()
endif()
