r"""
Installation requirements:
    pip install trame trame-vuetify trame-vtk vtk
"""
from pathlib import Path
import asyncio
from trame.app import get_server
from trame.decorators import TrameApp, change
from trame.widgets import vuetify, vtk as vtk_widgets
from trame.ui.vuetify import SinglePageLayout

from vtkmodules.vtkRenderingAnari import vtkAnariPass, vtkAnariRendererNode
from vtkmodules.vtkIOGeometry import vtkOBJReader
from vtkmodules.vtkRenderingCore import (
    vtkRenderer,
    vtkRenderWindow,
    vtkRenderWindowInteractor,
    vtkPolyDataMapper,
    vtkActor,
)

# VTK factory initialization
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleSwitch  # noqa
import vtkmodules.vtkRenderingOpenGL2  # noqa

# -----------------------------------------------------------------------------


@TrameApp()
class Cone:
    def __init__(self, server_or_name=None):
        self.server = get_server(server_or_name, client_type="vue2")
        self.server.cli.add_argument('-d', '--data')
        self.server.cli.add_argument('-l', '--anariLibrary', default='environment')
        self._vtk_rw = self._vtk_setup()
        self.ui = self._generate_ui()
        self.ctrl.on_client_connected.add_task(self._render_loop)

    @property
    def ctrl(self):
        return self.server.controller

    @property
    def state(self):
        return self.server.state

    @change("denoise")
    def on_denoise_change(self, denoise, **kwargs):
        self._rendererNode.SetUseDenoiser(denoise, self._renderer)
        self.ctrl.view_update()

    async def _render_loop(self):
        while True:
            self._vtk_rw.Render()
            self.ctrl.view_update()
            await asyncio.sleep(0.1)

    def _vtk_setup(self):
        args,_ = self.server.cli.parse_known_args()

        reader = vtkOBJReader()
        reader.SetFileName(str(Path(args.data).resolve()))

        renderer = vtkRenderer()
        renderWindow = vtkRenderWindow()

        anariPass = vtkAnariPass()
        vtkAnariRendererNode.SetLibraryName(args.anariLibrary, renderer)
        vtkAnariRendererNode.SetSamplesPerPixel(16, renderer)
        vtkAnariRendererNode.SetLightFalloff(.5, renderer)
        vtkAnariRendererNode.SetUseDenoiser(1, renderer)
        vtkAnariRendererNode.SetAmbientSamples(1, renderer)
        vtkAnariRendererNode.SetCompositeOnGL(1, renderer)
        self._rendererNode = vtkAnariRendererNode
        self._renderer = renderer

        renderer.SetPass(anariPass)
        renderWindow.AddRenderer(renderer)
        renderWindow.SetMultiSamples(0)
        renderWindow.OffScreenRenderingOn()

        renderWindowInteractor = vtkRenderWindowInteractor()
        renderWindowInteractor.SetRenderWindow(renderWindow)
        renderWindowInteractor.GetInteractorStyle().SetCurrentStyleToTrackballCamera()

        mapper = vtkPolyDataMapper()
        actor = vtkActor()
        mapper.SetInputConnection(reader.GetOutputPort())
        actor.SetMapper(mapper)
        renderer.AddActor(actor)
        renderer.ResetCamera()
        renderWindow.Render()

        return renderWindow

    def _generate_ui(self):
        with SinglePageLayout(self.server) as layout:
            layout.title.set_text("Trame demo")
            with layout.toolbar as toolbar:
                toolbar.dense = True
                vuetify.VSpacer()
                #vuetify.VSlider(
                #    v_model=("resolution", 6),
                #    min=3,
                #    max=60,
                #    step=1,
                #    hide_details=True,
                #    style="max-width: 300px;",
                #)
                vuetify.VSwitch(v_model=("denoise", True), label="denoise", hide_details=True)
                with vuetify.VBtn(icon=True, click=self.ctrl.view_reset_camera):
                    vuetify.VIcon("mdi-crop-free")

            with layout.content:
                with vuetify.VContainer(fluid=True, classes="pa-0 fill-height"):
                    view = vtk_widgets.VtkRemoteView(self._vtk_rw, interactive_ratio=1)
                    self.ctrl.view_update = view.update
                    self.ctrl.view_reset_camera = view.reset_camera

            return layout


def main(**kwargs):
    cone = Cone()
    cone.server.start(**kwargs)


if __name__ == "__main__":
    main()
