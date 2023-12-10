# JuceVLC: A fullscreen User Interface for VLC #

![https://github.com/maudoin/jucevlc/blob/master/web/images/vlc.jpg](https://github.com/maudoin/jucevlc/blob/master/web/images/vlc.jpg)

See old webpage here: http://jucevlc.sourceforge.net/index.html

![https://github.com/maudoin/jucevlc/blob/master/web/images/banner.jpg](https://github.com/maudoin/jucevlc/blob/master/web/images/banner.jpg)

Download [here](https://github.com/maudoin/jucevlc/releases)


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

## Reviews ##
* [![www.clubic.com](https://upload.wikimedia.org/wikipedia/fr/thumb/3/36/Logo-clubic.svg/langfr-225px-Logo-clubic.svg.png) www.clubic.com ](https://www.clubic.com/telecharger/actus-logiciels/article-619750-1-zoom-jucevlc.html)  (French)

    Portable, intuitif et complet comme VLC media player, JuceVLC offre une interface Media Center qui vous permettra de rechercher et incruster des sous-titres en ligne sur vos épisodes et films préférés.

* [![www.download3k.com](http://www.download3k.com/images/review9.gif) www.download3k.com](http://www.download3k.com/MP3-Audio-Video/Media-Players/Download-JuceVLC.html)

    When it's all said and done, I really like JuceVLC, and I'll definitely keep it on my computer for a while. That being said, I feel the initial UI could really throw off some basic PC users who wouldn't even think to try right-clicking the back arrow. In reality, that's still a very minor issue because it can be fixed easily, still a fantastic program you should definitely try.

* [![www.findmysoft.com](http://www.findmysoft.com/review2_4_JuceVLC_award.png) www.findmysoft.com](http://jucevlc.findmysoft.com/)

* [![100% CLEAN award granted by Softpedia](https://www.softpedia.com/_img/softpedia_100_free.png?2023_1) www.softpedia.com](http://www.softpedia.com/get/Multimedia/Video/Video-Players/JuceVLC.shtml)

* [![www.softmazing.com](http://www.softmazing.com/wp-content/uploads/2013/02/softmazing-amazing-software-award-150.jpg) www.softmazing.com](http://www.softmazing.com/a-fullscreen-gui-for-vlc-player-jucevlc/)

* [![www.lo4d.com ](http://www.lo4d.com/i/users_excellent.png) www.lo4d.com ](http://jucevlc.en.lo4d.com/)

  Overall and thanks to being powered by the immensely popular open source VLC, JuceVLC is able to provide a modern look combined with pure functionality. Perfect for those with larger screens.

* [connectfreegadget.com](http://translate.google.com/translate?hl=en&sl=auto&tl=en&u=http%3A%2F%2Fconnectfreegadget.com%2Fblog%2Fjucevlc_video_player_na_baze_vlc_obzor%2F2013-03-21-683) ([Russian](http://connectfreegadget.com/blog/jucevlc_video_player_na_baze_vlc_obzor/2013-03-21-683))
* [www.ilovefreesoftware.com](http://www.ilovefreesoftware.com/08/windows/mp3/jucevlc-video-player.html)
* [www.dinside.no](http://translate.google.com/translate?hl=en&sl=auto&tl=en&u=http%3A%2F%2Fwww.dinside.no%2F913273%2Fjucevlc-mediesenter-basert-paa-vlc) ([Norwegian](http://www.dinside.no/913273/jucevlc-mediesenter-basert-paa-vlc))
* [www.addictivetips.com](http://www.addictivetips.com/windows-tips/jucevlc-media-center-style-video-player-based-on-vlc/)

## Changelog ##

See [README.txt](https://github.com/maudoin/jucevlc/blob/master/README.txt)

## Screenshots ##

### Browse files ###

![https://github.com/maudoin/jucevlc/blob/master/web/screenshots/open.jpg](https://github.com/maudoin/jucevlc/blob/master/web/screenshots/open.jpg)

### On Screen Display ###

![https://github.com/maudoin/jucevlc/blob/master/web/screenshots/playing.jpg](https://github.com/maudoin/jucevlc/blob/master/web/screenshots/playing.jpg)

### Subtitles management and adjustment ###
![https://github.com/maudoin/jucevlc/blob/master/web/screenshots/subtitles.jpg](https://github.com/maudoin/jucevlc/blob/master/web/screenshots/subtitles.jpg)

### Quick adjustement : audio volume, subtitles delay, audio delay, playback speed, disable ###

![https://github.com/maudoin/jucevlc/blob/master/web/screenshots/quickAdjust.jpg](https://github.com/maudoin/jucevlc/blob/master/web/screenshots/quickAdjust.jpg)

### Video settings ###
![https://github.com/maudoin/jucevlc/blob/master/web/screenshots/imageSettings.jpg](https://github.com/maudoin/jucevlc/blob/master/web/screenshots/imageSettings.jpg)


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