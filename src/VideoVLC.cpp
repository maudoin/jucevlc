/***************************************************************************
 * Name: VideoVLC.cpp
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

#include "VideoVLC.h"

VideoVLC::VideoVLC ()
         :Component ("VideoVLC")
{
    setSize (400, 200);

    vlc_visor=new Component("VisorVLC");
    vlc_visor->setOpaque(true);
    vlc_visor->addToDesktop(ComponentPeer::windowIsTemporary);
    isLoadMedia=false;
}
VideoVLC::~VideoVLC()
{
    delete vlc_visor;
}
void VideoVLC::paint (Graphics& g)
{
    g.fillAll (Colours::black);
}
void VideoVLC::resized()
{
    if(getState()==STATE_PLAYING || getState()==STATE_PAUSED)
        vlc_visor->setBounds(getScreenX(),getScreenY(),getWidth(),getHeight());
}
void VideoVLC::timerCallback()
{
    if (getState()==STATE_ENDED || getState()==STATE_STOPPING)
    {
        //stopTimer();
        vlc_visor->setVisible(false);
        stop();
        return;
    }

    if(getState()==STATE_PLAYING && vlc_visor->isVisible()==false){
        vlc_visor->setVisible(true);
    }

    if(getPeer()->isMinimised()==false){
      getPeer()->toBehind(vlc_visor->getPeer());
    }
    if(getPeer()->isFocused()==true){
      vlc_visor->getPeer()->toFront(false);
    }
}
void VideoVLC::play()
{
    if (isLoadMedia==false || getState()==STATE_PLAYING)
    {
        return;
    }
    vlc_visor->setBounds(getScreenX(),getScreenY(),getWidth(),getHeight());
    getPeer()->getComponent().addComponentListener(this);

    libvlc_media_player_play (vlc_mplayer);
    startTimer(1);
}
void VideoVLC::stop()
{
    if (getState()!=STATE_PLAYING && getState()!=STATE_PAUSED)
    {
        return;
    }
    getPeer()->getComponent().removeComponentListener(this);

    libvlc_media_player_stop(vlc_mplayer);

}
void VideoVLC::pause()
{
    if (getState()!=STATE_PLAYING)
    {
        return;
    }
    libvlc_media_player_pause(vlc_mplayer);
}
void VideoVLC::setRate(float p_rate)
{
    if (isLoadMedia==false)
    {
        return;
    }
    libvlc_media_player_set_rate(vlc_mplayer,p_rate );
}
float VideoVLC::getRate()
{
    if (isLoadMedia==false)
    {
        return -1;
    }
    float vr=libvlc_media_player_get_rate(vlc_mplayer);
    return vr;
}
void VideoVLC::setPosition(int p_posi)
{
    if (isLoadMedia==false)
    {
        return;
    }
    libvlc_media_t *curMedia = libvlc_media_player_get_media (vlc_mplayer);
    if (curMedia == NULL)
        return;

    float pos=(float)(p_posi)/(float)10000;
    libvlc_media_player_set_position (vlc_mplayer,pos);
    //raise(&vlc_except);
}
int VideoVLC::getPosition()
{
    if (isLoadMedia==false)
    {
        return -1;
    }
    libvlc_media_t *curMedia = libvlc_media_player_get_media (vlc_mplayer);
    if (curMedia == NULL)
        return -1;

    float pos=libvlc_media_player_get_position (vlc_mplayer);
    int vr=(int)(pos*(float)(10000));
    return vr;
}
void VideoVLC::loadMedia(String pmedia)
{
    if (vlc_mplayer !=0)
    {
        stop();
    }

    const char * const vlc_args[] =
    {
        "-I", "dummy",
        "--ignore-config",
        //"--novideo",
        //"--fullscreen",
        //"-crop-geometry","30x30+10+10",
        "--width","20",
        "--height","20",
        "--no-video-title-show",
        //"--plugin-path=plugins\\"
    };

    vlc_instan = libvlc_new (sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);

    vlc_media = libvlc_media_new_location (vlc_instan,pmedia.getCharPointer());

    vlc_mplayer = libvlc_media_player_new_from_media (vlc_media);
    libvlc_media_release (vlc_media);

    //creando eventos del reproductor
    //vlc_eventManager=libvlc_media_player_event_manager(vlc_mplayer);
    //libvlc_event_attach(vlc_eventManager,libvlc_MediaPlayerPositionChanged,callback,NULL);

    if (SystemStats::getOperatingSystemName()==("Linux"))
    {
        libvlc_media_player_set_xwindow(vlc_mplayer,(uint32_t)vlc_visor->getWindowHandle());
    }
    else if(SystemStats::getOperatingSystemName()==("MacOSX")){
        libvlc_media_player_set_agl(vlc_mplayer,(uint32_t)vlc_visor->getWindowHandle());
    }else{
        libvlc_media_player_set_hwnd(vlc_mplayer,vlc_visor->getWindowHandle());
    }
    isLoadMedia=true;
}
void VideoVLC::setVolume(int p_vol)
{
    if (isLoadMedia==false)
    {
        return;
    }
    if (p_vol>100)
    {
        p_vol=100;
    }
    if (p_vol<0)
    {
        p_vol=0;
    }
    libvlc_audio_set_volume(vlc_mplayer,p_vol);
}
int VideoVLC::getVolume()
{
    if (isLoadMedia==false)
    {
        return -1;
    }
    int vr=libvlc_audio_get_volume(vlc_mplayer);
    return vr;
}
int VideoVLC::getState()
{
    if (isLoadMedia==false)
    {
        return STATE_STOPPING;
    }
    return libvlc_media_get_state(vlc_media);
}
float VideoVLC::getDuration()
{
    if (isLoadMedia==false)
    {
        return 0;
    }
    return libvlc_media_player_get_length(vlc_mplayer);
}
void VideoVLC::componentMovedOrResized(Component &  component,bool wasMoved, bool wasResized){
    resized();
}
void VideoVLC::componentVisibilityChanged(Component &  component){
     resized();
}
/*
void VideoVLC::callback( const libvlc_event_t *ev, void *param )
{
    if( ev->type == libvlc_MediaPlayerStopped)
    {
        vlc_visor->setVisible(false);
    }
}*/

