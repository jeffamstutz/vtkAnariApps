import vtk

# Create a sphere source
sphere = vtk.vtkSphereSource()
sphere.SetCenter(0.0, 0.0, 0.0)
sphere.SetRadius(5.0)

# Create a mapper
mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(sphere.GetOutputPort())

# Create an actor
actor = vtk.vtkActor()
actor.SetMapper(mapper)

# Create a renderer
renderer = vtk.vtkRenderer()
renderer.AddActor(actor)
renderer.SetBackground(0.1, 0.2, 0.4)  # Background color (RGB)

# Create a render window with offscreen rendering enabled
renderWindow = vtk.vtkRenderWindow()
renderWindow.SetOffScreenRendering(1)  # Enable offscreen rendering
renderWindow.AddRenderer(renderer)
renderWindow.SetSize(800, 800)  # Set render window size

# Create a render pass using vtkAnariPass
anariPass = vtk.vtkAnariPass()
ad = anariPass.GetAnariDevice()
ad.SetupAnariDeviceFromLibrary("helide", "default")
ar = anariPass.GetAnariRenderer()
ar.SetParameter("ambientRadiance", 1.0)

# Assign the ANARI render pass to the renderer
renderer.SetPass(anariPass)

vtk.vtkAnariSceneGraph.SetCompositeOnGL(renderer, 1)

# Render the scene
renderWindow.Render()

# Create a window-to-image filter to capture the render
windowToImageFilter = vtk.vtkWindowToImageFilter()
windowToImageFilter.SetInput(renderWindow)
windowToImageFilter.SetScale(1)  # Image quality scaling
windowToImageFilter.SetInputBufferTypeToRGBA()  # Capture full color buffer
windowToImageFilter.ReadFrontBufferOff()  # Read from the back buffer
windowToImageFilter.Update()

# Save the image to a PNG file
writer = vtk.vtkPNGWriter()
writer.SetFileName("anari_render.png")
writer.SetInputConnection(windowToImageFilter.GetOutputPort())
writer.Write()
