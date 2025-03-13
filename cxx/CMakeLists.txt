## Copyright 2024 Jefferson Amstutz
## SPDX-License-Identifier: Apache-2.0

option(BUILD_IMGUI_VIEWER "Build vtkAnariImguiViewer?" OFF)
if (BUILD_IMGUI_VIEWER)
  add_subdirectory(vtkAnariImguiViewer)
endif()
add_subdirectory(vtkAnariObjViewer)
