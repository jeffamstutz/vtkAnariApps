// Copyright 2024 Jefferson Amstutz
// SPDX-License-Identifier: Apache-2.0

#include "anari_viewer/Application.h"
#include "anari_viewer/windows/LightsEditor.h"
#include "anari_viewer/windows/SceneSelector.h"
#include "anari_viewer/windows/Viewport.h"
// anari
#include <anari_test_scenes.h>
// std
#include <iostream>

static const bool g_true = true;
static bool g_verbose = false;
static bool g_useDefaultLayout = true;
static std::string g_libraryName = "environment";
static anari::Device g_device = nullptr;

extern const char *getDefaultUILayout();

static void statusFunc(const void *userData,
    ANARIDevice device,
    ANARIObject source,
    ANARIDataType sourceType,
    ANARIStatusSeverity severity,
    ANARIStatusCode code,
    const char *message)
{
  const bool verbose = userData ? *(const bool *)userData : false;
  if (severity == ANARI_SEVERITY_FATAL_ERROR) {
    fprintf(stderr, "[FATAL][%p] %s\n", source, message);
    std::exit(1);
  } else if (severity == ANARI_SEVERITY_ERROR) {
    fprintf(stderr, "[ERROR][%p] %s\n", source, message);
  } else if (severity == ANARI_SEVERITY_WARNING) {
    fprintf(stderr, "[WARN ][%p] %s\n", source, message);
  } else if (verbose && severity == ANARI_SEVERITY_PERFORMANCE_WARNING) {
    fprintf(stderr, "[PERF ][%p] %s\n", source, message);
  } else if (verbose && severity == ANARI_SEVERITY_INFO) {
    fprintf(stderr, "[INFO ][%p] %s\n", source, message);
  } else if (verbose && severity == ANARI_SEVERITY_DEBUG) {
    fprintf(stderr, "[DEBUG][%p] %s\n", source, message);
  }
}

namespace vtk_imgui {

struct AppState
{
  anari_viewer::manipulators::Orbit manipulator;
  anari::Device device{nullptr};
};

// Application definition /////////////////////////////////////////////////////

struct Application : public anari_viewer::Application
{
  Application() = default;
  ~Application() override = default;

  anari_viewer::WindowArray setupWindows() override
  {
    anari_viewer::ui::init();

    // ANARI //

    m_state.device = g_device;

    // ImGui //

    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.5f;
    io.IniFilename = nullptr;

    if (g_useDefaultLayout)
      ImGui::LoadIniSettingsFromMemory(getDefaultUILayout());

    auto *viewport = new anari_viewer::windows::Viewport(g_device, "Viewport");
    viewport->setManipulator(&m_state.manipulator);

    auto *leditor = new anari_viewer::windows::LightsEditor(g_device);

    auto *sselector = new anari_viewer::windows::SceneSelector();
    sselector->setCallback([=](const char *category, const char *scene) {
      try {
        auto s = anari::scenes::createScene(g_device, category, scene);
        anari::scenes::commit(s);
        auto w = anari::scenes::getWorld(s);
        viewport->setWorld(w, true);
        leditor->setWorlds({w});
        sselector->setScene(s);
      } catch (const std::runtime_error &e) {
        printf("%s\n", e.what());
      }
    });

    anari_viewer::WindowArray windows;
    windows.emplace_back(viewport);
    windows.emplace_back(leditor);
    windows.emplace_back(sselector);

    return windows;
  }

  void uiFrameStart() override
  {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("print ImGui ini")) {
          const char *info = ImGui::SaveIniSettingsToMemory();
          printf("%s\n", info);
        }

        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }
  }

  void teardown() override
  {
    anari::release(m_state.device, m_state.device);
    anari_viewer::ui::shutdown();
  }

 private:
  AppState m_state;
};

} // namespace vtk_imgui

///////////////////////////////////////////////////////////////////////////////

static void printUsage()
{
  std::cout << "./anariViewer\n"
            << "   [{--help|-h}]\n"
            << "   [{--verbose|-v}]]\n"
            << "   [{--library|-l} <ANARI library>]\n";
}

static void parseCommandLine(int argc, char *argv[])
{
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "-v" || arg == "--verbose")
      g_verbose = true;
    if (arg == "--help" || arg == "-h") {
      printUsage();
      std::exit(0);
    } else if (arg == "--noDefaultLayout")
      g_useDefaultLayout = false;
    else if (arg == "-l" || arg == "--library")
      g_libraryName = argv[++i];
  }
}

int main(int argc, char *argv[])
{
  // Handle command line //

  parseCommandLine(argc, argv);

  // Create ANARI device //

  auto library =
      anariLoadLibrary(g_libraryName.c_str(), statusFunc, &g_verbose);
  if (!library)
    return 1;

  g_device = anariNewDevice(library, "default");
  if (!g_device)
    return 1;

  // Create + run app //

  vtk_imgui::Application app;
  app.run(1920, 1200, "VTK-ANARI Viewer");

  // Cleanup //

  anari::unloadLibrary(library);

  return 0;
}