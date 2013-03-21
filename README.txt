**Documentation**
==================


Portable Application installation
------------------

Extract the archive on any folder and run **JuceVLC.exe**
All files except JuceVLC.exe are taken from VLC so you can paste JuceVLC.exe in an existing VLC installation (2.0.0 and up).
The application can be executed from a usb key or an external drive as its settings are stored along the exe file
**Under Windows 7 it is not recommanded to extract the application in "Program Files" as Windows won't allow the settings to written.**


Menu
------------------
The main menu is accessible by right clicking on the video or using the third playback control bar icon (folder in a blue dot).

The main menu can be hidden bu left clicking on the video or using the third playback control bar icon (red cross over a folder in a blue dot).

Playback control bar
------------------

The playback control bar is shown when a movie is playing. I disappear if the mouse is not moving and brought back on mouse movement.

The top slider controls the video position.
The smaller bottom slider function can be selected using the left icon:
It controls:
* audio volume
* subtitles delay
* audio delay
* playback speed
* hidden replaced by the current time and estimated movie en time

**Changelog**
==================

0.5 (2013/03/21):
- Installer
- Localization support (English/French for now) -> look at "France.lang" to add other languages
- Automatic subtitles position mode (always above controls)

0.4 (2013/03/17):
-Save last 30 videos progress
-Subtitles position slider (avoids subtitles being hidden by controls)
-Smarter OSD Controls disappearing
-Fixed moved window flickering
-Added new configuration option (subtitles size, video deinterlace, video acceleration, video quality). They affect the next played video (added Apply option for this)

0.3 (2013/03/16):
-Fixed windowed mode being activated on video stop
-Fixed window dragging with a new title bar
-Added font size selection
-Added new icons

0.2 (2013/03/15):
-Fixed disappearing controls on pause or slider movement
-Added video adjustments (contrast, ...)
-Moved windowed/fullscreen options to player sub-menu

0.1 (2013/03/04):
-First release 