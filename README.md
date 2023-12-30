# JuceVLC: A fullscreen User Interface for VLC #

## Summary ##

JuceVLC is VLC with a simple Media Center like fullscreen User Interface.

![https://github.com/maudoin/jucevlc/blob/master/web/images/banner.jpg](https://github.com/maudoin/jucevlc/blob/master/web/images/banner.jpg)

Download [here](https://github.com/maudoin/jucevlc/releases)

## Goal ##

Browse and watch movies from your couch with a wireless mouse.

## Features ##

![https://github.com/maudoin/jucevlc/blob/master/icons/vlc.svg](https://github.com/maudoin/jucevlc/blob/master/icons/vlc.svg)

  * Browse local files easily and adjust settings using On Screen Display (O.S.D.), no tiny system file selection dialog
  * Big, customizable fonts size and no popup dialogs
  * Subtitles selection and synchronization using slider via O.S.D.
  * Search,Download,Extract and Load subtitles from [opensubtitles.org](http://www.opensubtitles.org)
  * Keep last and favorite video folders, as well as the last thirty videos positions
  * Does not mess up your system: settings are stored in the application folder, not the registry (even through the installer)
  * Portable: Installer can be run multiple times:
    * once for your computer, activating shortcut creation,
    * and again for your usb key or external hard drive. (you can uncheck the shortcuts creation when installing)
  * Since it does not store anything in the registry subsequent installs won't affect the previous ones (just don't let the installer create shortcuts)
  * JuceVLC application uses regular VLC core libraries/plugins: You can usually paste future VLC minor updates along JuceVLC to update the core video player without any JuceVLC update
  * The "space-key" allows your relatives to play/pause your video if they want to interrupt your watching session;-)

See old webpage here: http://jucevlc.sourceforge.net/index.html

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

The project relies on multi-platform libraries **could** also compile on Linux / MacOS environments

Tools / Libraries
------------------
* **clang-cli 17.0.6** (for Windows) <https://github.com/llvm/llvm-project/releases/tag/llvmorg-17.0.6>
* **JUCE 7.0.9** (**repository submodule**) <https://juce.com/>
* **LibVLC 3.0.8** (**repository submodule**, .lib files extracted from windows 64bits installed  .dll) <http://www.videolan.org/vlc/index.html>
* **CMake 3.15** <https://cmake.org/download>
* **NSIS 3.09** (for Windows) <https://sourceforge.net/projects/nsis>
* **NSIS FindProcDLL plugin** <https://nsis.sourceforge.io/FindProcDLL_plug-in>
* **Visual code** (facultative)

VLC/JUCE Submodules update
---------------------------

git submodule update --init --recursive
