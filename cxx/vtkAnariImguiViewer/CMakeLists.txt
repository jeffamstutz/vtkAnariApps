## Copyright 2024 Jefferson Amstutz
## SPDX-License-Identifier: Apache-2.0

project(vtkAnariImguiViewer)
add_executable(${PROJECT_NAME} ui_layout.cpp vtkAnariImguiViewer.cpp)
target_link_libraries(${PROJECT_NAME} anari::anari_viewer ${VTK_LIBRARIES})
