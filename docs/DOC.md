
# Capture Requests

When `flameshot gui` is called, a `CaptureRequest` object is created. The
object contains a set of tasks depending on the passed CLI options. If no
options are passed, the capture request has no tasks.

# Screenshot saving

Screenshots are saved using the ScreenshotSaver class. This class provides two
functions to do this:
- saveToFilesytem - saves to the specified path
- saveToFilesytemGUI - opens a save dialog
