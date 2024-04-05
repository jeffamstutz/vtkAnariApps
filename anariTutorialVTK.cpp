// Copyright 2024 Jefferson Amstutz
// SPDX-License-Identifier: Apache-2.0

// vtk
#include "vtkAnariPass.h"
#include "vtkAnariRendererNode.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkLogger.h"
#include "vtkNew.h"
#include "vtkOBJReader.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
// anari
#include "anari/anari_cpp.hpp"
#include "anari/anari_cpp/ext/linalg.h"

using namespace anari::math;

struct IdleCallback : vtkCommand
{
  vtkTypeMacro(IdleCallback, vtkCommand);

  static IdleCallback *New()
  {
    return new IdleCallback;
  }

  void Execute(vtkObject *vtkNotUsed(caller),
      unsigned long vtkNotUsed(eventId),
      void *vtkNotUsed(callData))
  {
    renderWindow->Render();
#if 0
    static int cnt = 0;
    std::cout << cnt++ << '\n';
#endif
  }

  vtkRenderWindow *renderWindow;
};

int main(int argc, char *argv[])
{
  vtkLogger::SetStderrVerbosity(vtkLogger::Verbosity::VERBOSITY_WARNING);

  vtkNew<vtkOBJReader> reader;
  reader->SetFileName(argv[1]);
  reader->Update();
  reader->Print(cout);

  // The mapper / ray cast function know how to render the data
  vtkNew<vtkPolyDataMapper> polyMapper;
  polyMapper->SetInputConnection(reader->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(polyMapper);

  vtkNew<vtkRenderer> ren1;
  ren1->AddActor(actor);

  ren1->SetBackground(0.3, 0.3, 0.4);
  ren1->ResetCamera();

  // Attach ANARI render pass
  vtkNew<vtkAnariPass> anariPass;
  ren1->SetPass(anariPass);

  vtkAnariRendererNode::SetLibraryName("environment", ren1);
  vtkAnariRendererNode::SetSamplesPerPixel(1, ren1);
  vtkAnariRendererNode::SetLightFalloff(.5, ren1);
  vtkAnariRendererNode::SetUseDenoiser(0, ren1);
  vtkAnariRendererNode::SetAmbientSamples(1, ren1);
  vtkAnariRendererNode::SetCompositeOnGL(1, ren1);

  // Create the renderwindow
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetMultiSamples(0);
  renderWindow->AddRenderer(ren1);
  renderWindow->SetSize(1600, 900);

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renderWindow);
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  iren->SetInteractorStyle(style);

  vtkNew<IdleCallback> idleCallback;
  idleCallback->renderWindow = renderWindow;
  iren->CreateRepeatingTimer(1);
  iren->AddObserver(vtkCommand::TimerEvent, idleCallback);
  iren->Start();

  return 0;
}
