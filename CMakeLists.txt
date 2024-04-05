## Copyright 2024 Jefferson Amstutz
## SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.17)

project(anariTutorialVTK)

enable_language(CXX)

find_package(VTK REQUIRED COMPONENTS
  CommonColor
  CommonCore
  CommonDataModel
  FiltersCore
  FiltersGeometry
  FiltersModeling
  InteractionStyle
  IOGeometry
  RenderingContextOpenGL2
  RenderingCore
  RenderingFreeType
  RenderingGL2PSOpenGL2
  RenderingOpenGL2
  RenderingAnari
)

add_executable(${PROJECT_NAME} anariTutorialVTK.cpp)
target_link_libraries(${PROJECT_NAME} anari::anari ${VTK_LIBRARIES}) #util)
target_include_directories(${PROJECT_NAME} PRIVATE ../util)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ../external)
