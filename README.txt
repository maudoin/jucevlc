**Documentation**
==================


Portable Application installation
------------------

Extract the archive on any folder and run **JuceVLC.exe**
You can use the installer multiple times:
* once for your computer, activating shortcut creation, 
* and again for your usb key or external hard drive.
Since it does not store anything in the registry subsequent installs won't affect the previous ones (except for optionnal shortcut creation obviously)

All files except JuceVLC.exe are taken from VLC so you can paste JuceVLC.exe in an existing VLC installation (2.0.0 and up).
The application can be executed from a usb key or an external drive as its settings are stored along the exe file

**Under Windows 7 it is not recommanded to extract the application in "Program Files" as Windows won't allow the settings to written.**


Menu
------------------
The main menu is accessible by right clicking on the video or using the third playback control bar icon (folder in a blue dot).

The main menu can be hidden bu left clicking on the video or using the third playback control bar icon (red cross over a folder in a blue dot).

WARNING: Menu options prefixed with a star are applied to the next played video

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

0.67 (2013/05/10):
- Save current crop ratio as it is not video but screen dependant
- Auto purge missing favorite folders on available drives
- Fixed secondary slider (audio offset...) reset after using arrows
- Fixed video position no recovered from current playlist selection
- Hide menu instead of shriking it on file open

0.66 (2013/04/11):
- libVLC 2.0.6

0.65 (2013/04/06):
- File listing sorting improvement
- Auxiliary slider reset button
- Fullscreen button
- Auxiliary slider function selection icons improvement

0.6 (2013/03/24):
- Audio equaliser (presets)
- Advanced subtitle configuration (colors, opacity,...) WARNING they are applied to the next played video
- File browsing improvement (sorting and icons)
- non disappearing controls fix

0.5 (2013/03/21):
- Installer (copying the new exe over previous installs still works) The application is still as portable as previously, the installer behaves just like extracting an archive, plus optionnal shortcuts creation.
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