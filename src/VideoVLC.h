/***************************************************************************
 * Name: VideoVLC.h
 * Purpose: Component JUCE for LibVLC .
 * Primary Author & Head Developer: Yosvanis Cruz Cruz.(yosvaniscc@gmail.com)
 * Original Idea Author: Jason Ostrom (Jpo@pobox.com)
 * Created: 2010-03-28
 * Updated: 2010-04-16
 * Copyright: 2010 por CEINFED...para Haeduc
 * License: GNU General Public License.
 * Nota:
 *      development forum the class: En http://www.rawmaterialsoftware.com/viewtopic.php?f=2&t=5202
 *        -Yosvanis Cruz Cruz is user yosvaniscc
 *        -Jason Ostrom is user puckhead
 *      Other used sources: http://wiki.videolan.org/Libvlc
 ***************************************************************************/
#ifndef __JUCER_HEADER_VIDEOVLC__
#define __JUCER_HEADER_VIDEOVLC__
//[Headers]
#include <vlc/vlc.h>
#include <vlc/libvlc.h>
#include "juce.h"
//[/Headers]
class VideoVLC  : public juce::Component,
                  private Timer,
                  private ComponentListener
                                {

public:
    //==============================================================================
    VideoVLC ();
    ~VideoVLC();

    //==============================================================================
    enum{
    STATE_IDLE_CLOSE,
    STATE_OPENING,
    STATE_BUFFERING,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_STOPPING,
    STATE_ENDED,
    STATE_ERROR,
    };
    //[UserMethods]     -- You can add your own custom methods in this section.
    void play();
    void stop();
    void pause();
    void setPosition(int p_posi);
    int getPosition();
    void setRate(float p_rate);
    float getRate();
    void setVolume(int p_vol);
    int getVolume();
    void loadMedia(String media);
    bool isLoadMedia;
    int getState();
    float getDuration();
    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    void paint (Graphics& g);
    void resized();
    void timerCallback();

    //void callback(const libvlc_event_t *ev, void *param );

    void componentMovedOrResized(Component& component,bool wasMoved, bool wasResized);
    void componentVisibilityChanged(Component& component);

    Component *vlc_visor;
    libvlc_instance_t *vlc_instan;
    libvlc_media_player_t *vlc_mplayer;
    libvlc_media_t *vlc_media;
    //libvlc_event_manager_t *vlc_eventManager;
    //struct libvlc_rectangle_t vlc_rectangle;
    //[/UserVariables]
    //==============================================================================

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    VideoVLC (const VideoVLC&);
    const VideoVLC& operator= (const VideoVLC&);
};
#endif   // __JUCER_HEADER_VIDEOVLC__
