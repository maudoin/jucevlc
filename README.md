# JuceVLC: A fullscreen User Interface for VLC #

![http://jucevlc.sourceforge.net/images/vlc.jpg](http://jucevlc.sourceforge.net/images/vlc.jpg)

# see full webpage here: http://jucevlc.sourceforge.net/ #

![http://jucevlc.googlecode.com/hg/web/images/banner.jpg](http://jucevlc.googlecode.com/hg/web/images/banner.jpg)

Download here: http://sourceforge.net/projects/jucevlc/files/latest/download



## Summary ##

JuceVLC is VLC with a simple Media Center like fullscreen User Interface.

## Goal ##

Browse and watch movies from your couch with a wireless mouse.

## Features ##

  * Browse local or remote (via UPNP) files and adjust settings using On Screen Display (O.S.D.)
  * Frontpage movies menu with automatically downloaded posters (from http://www.omdbapi.com) or generated thumbnails (no setup required)
  * Big, customizable fonts size and no tiny popup dialogs
  * Subtitles selection and synchronization using slider via O.S.D.
  * Search,Download,Extract and Load subtitles from [opensubtitles.org](http://www.opensubtitles.org)
  * Keep last and favorite video folders, as well as the last thirty videos positions
  * Does not mess up your system: settings are stored in the application folder
  * Portable: Paste is on any usb key or external hard drive
  * You can use the installer multiple times:
    * once for your computer, activating shortcut creation,
    * and again for your usb key or external hard drive.
> > Since it does not store anything in the registry subsequent installs won't affect the previous ones (except shortcut creation)
  * JuceVLC application uses regular VLC core libraries/plugins: You can paste future VLC versions directory along JuceVLC to update the core video player without any JuceVLC update

## Changelog ##

README.txt
http://jucevlc.sourceforge.net/site.html#news

## Screenshots ##

### Browse files ###
![http://jucevlc.googlecode.com/hg/web/screenshots/open.jpg](http://jucevlc.googlecode.com/hg/web/screenshots/open.jpg)

### On Screen Display ###
![http://jucevlc.googlecode.com/hg/web/screenshots/playing.jpg](http://jucevlc.googlecode.com/hg/web/screenshots/playing.jpg)

### Subtitles management and adjustment ###
![http://jucevlc.googlecode.com/hg/web/screenshots/subtitles.jpg](http://jucevlc.googlecode.com/hg/web/screenshots/subtitles.jpg)

### Quick adjustement : audio volume, subtitles delay, audio delay, playback speed, disable ###

![http://jucevlc.googlecode.com/hg/web/screenshots/quickAdjust.jpg](http://jucevlc.googlecode.com/hg/web/screenshots/quickAdjust.jpg)

### Video settings ###
![http://jucevlc.googlecode.com/hg/web/screenshots/imageSettings.jpg](http://jucevlc.googlecode.com/hg/web/screenshots/imageSettings.jpg)


**Developpers / Building tips**
==================

The project relies on multi-platform libraries / GNU gcc compiler and should also compile on Linux / MacOS environments

Tools / Libraries
------------------
* **codeblocks-13.12** (for Windows)
* **Mingw32 4.8** gcc compiler (for Windows)
* **Juce 2.1.3** (**included in repository**) 
<http://rawmaterialsoftware.com/downloads.php>
* **LibVLC 2.2.0** (**included in repository**, taken from VLC distributions sdk folder) <http://www.videolan.org/vlc/index.html>
* **Boost 1.56** includes (to be extracted in **/boost** folder) <https://sourceforge.net/projects/boost/files/boost/1.56.0/>
* **NSIS 2.46** for the windows installer
* **UPX 3.08**

**Boost 1.56 regexp lib build batch file for mingw: (if mingw is installed in e:\dev\mingw)**

set MINGW=e:\dev\mingw\bin
set PATH=%PATH%;%MINGW%
%~d0
cd %~dp0
bootstrap.bat mingw --toolset-root=%MINGW%
b2  --toolset=gcc regex release link=static threading=multi runtime-link=static


**Boost 1.56 64bits regexp lib build batch file for mingw: (if mingw is installed in E:\dev\x64-4.8.1-release-win32-seh-rev5\mingw64)**

set MINGW=E:\dev\x64-4.8.1-release-win32-seh-rev5\mingw64\bin
set PATH=%PATH%;%MINGW%
%~d0
cd %~dp0
bootstrap.bat mingw --toolset-root=%MINGW%
b2 address-model=64 --toolset=gcc regex release link=static threading=multi runtime-link=static
pause