## Copyright 2024 Jefferson Amstutz
## SPDX-License-Identifier: Apache-2.0

project(vtkAnariObjViewer)
add_executable(${PROJECT_NAME} vtkAnariObjViewer.cpp)
target_link_libraries(${PROJECT_NAME} anari::anari ${VTK_LIBRARIES})
