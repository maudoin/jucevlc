/************************************************************************
    This file is part of VLCWrapper.
    
    File:    VLCWrapper.cpp
    Desc.:   VLCWrapper Implementation.

	Author:  Alex Skoruppa
	Date:    08/10/2009
	Updated: 03/12/2012
	eM@il:   alex.skoruppa@googlemail.com

	VLCWrapper is distributed under the Code Project Open License (CPOL).

	You should have received a copy of the Code Project Open License
	along with VLCWrapper.  If not, see <http://www.codeproject.com/info/cpol10.aspx>.
************************************************************************/
#include "VLCWrapper.h"
#include "vlc\vlc.h"
#include "vlc\libvlc_events.h"
#include <stdio.h>

static void HandleVLCEvents(const libvlc_event_t* pEvent, void* pUserData)
{
    EventCallBack* cb = reinterpret_cast<EventCallBack*>(pUserData); 
	if(!cb)
	{
		return;
	}
 
    switch(pEvent->type)
    {
		case libvlc_MediaPlayerTimeChanged:
		    cb->timeChanged();
            break;
	   case libvlc_MediaPlayerPlaying :
		    cb->started();
            break;
	   case libvlc_MediaPlayerPaused:
		    cb->paused();
            break;
	   case libvlc_MediaPlayerStopped:
		    cb->stopped();
            break;
	} 
}

static void *lock(void *pUserData, void **p_pixels)
{
    DisplayCallback* cb = reinterpret_cast<DisplayCallback*>(pUserData); 
	return cb?cb->lock( p_pixels):NULL;
}

static void unlock(void *pUserData, void *id, void *const *p_pixels)
{
    DisplayCallback* cb = reinterpret_cast<DisplayCallback*>(pUserData); 
	if(cb)
		cb->unlock( id, p_pixels);
}

static void display(void *pUserData, void *id)
{
    DisplayCallback* cb = reinterpret_cast<DisplayCallback*>(pUserData); 
	if(cb)
		cb->display(id);
}

VLCWrapper::VLCWrapper(void)
:	pVLCInstance_(0),
	pMediaPlayer_(0),
	pMedia_(0),
    pEventManager_(0)
{
	const char * const vlc_args[] = {
		"-I", "dumy",      // No special interface
		"--ignore-config", // Don't use VLC's config
        "--no-video-title-show",
		"--plugin-path=./plugins" };

	// init vlc modules, should be done only once
	pVLCInstance_ = libvlc_new (0, NULL);//sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
     
    // Create a media player playing environement
    pMediaPlayer_ = libvlc_media_player_new(pVLCInstance_);

    // Create an event manager for the player for handling e.g. time change events
    pEventManager_ = libvlc_media_player_event_manager(pMediaPlayer_);
}
std::string VLCWrapper::getInfo() const
{
	return libvlc_get_version();
}
VLCWrapper::~VLCWrapper(void)
{
	SetEventCallBack(0);
	SetDisplayCallback(0);
    // Free the media_player
    libvlc_media_player_release (pMediaPlayer_);
	libvlc_release (pVLCInstance_);
}

void VLCWrapper::SetDisplayCallback(DisplayCallback* cb)
{
	if(cb)
	{
	    libvlc_video_set_callbacks(pMediaPlayer_, lock, unlock, display, cb);
	}
}
	 	
 
void VLCWrapper::SetEventCallBack(EventCallBack* cb)
{
	if(cb)
	{
	    libvlc_event_attach (pEventManager_, libvlc_MediaPlayerTimeChanged, HandleVLCEvents, cb);
	    libvlc_event_attach (pEventManager_, libvlc_MediaPlayerPlaying, HandleVLCEvents, cb);
	    libvlc_event_attach (pEventManager_, libvlc_MediaPlayerPausableChanged, HandleVLCEvents, cb);
	    libvlc_event_attach (pEventManager_, libvlc_MediaPlayerPaused, HandleVLCEvents, cb);
	    libvlc_event_attach (pEventManager_, libvlc_MediaPlayerStopped, HandleVLCEvents, cb);
	}
	else
	{
		libvlc_event_detach (pEventManager_, libvlc_MediaPlayerTimeChanged, HandleVLCEvents, cb);
	    libvlc_event_detach (pEventManager_, libvlc_MediaPlayerPlaying, HandleVLCEvents, cb);
	    libvlc_event_detach (pEventManager_, libvlc_MediaPlayerPausableChanged, HandleVLCEvents, cb);
	    libvlc_event_detach (pEventManager_, libvlc_MediaPlayerPaused, HandleVLCEvents, cb);
	    libvlc_event_detach (pEventManager_, libvlc_MediaPlayerStopped, HandleVLCEvents, cb);
	}
}
void VLCWrapper::SetBufferFormat(int imageWidth, int imageHeight, int imageStride)
{
	libvlc_video_set_format(pMediaPlayer_, "RV24", imageWidth, imageHeight, imageStride);
}
void VLCWrapper::SetOutputWindow(void* pHwnd)
{
    // Set the output window    
	libvlc_media_player_set_hwnd(pMediaPlayer_, pHwnd);
}

void VLCWrapper::Play()
{
	// play the media_player
    libvlc_media_player_play (pMediaPlayer_);
}

void VLCWrapper::Pause()
{
	// Pause playing
    libvlc_media_player_pause (pMediaPlayer_);
}

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
bool VLCWrapper::isPaused()
{
    return libvlc_media_player_get_state(pMediaPlayer_) == libvlc_Paused;
}
bool VLCWrapper::isPlaying()
{
    return libvlc_media_player_get_state(pMediaPlayer_) == libvlc_Playing;
}

bool VLCWrapper::isStopping()
{
	//  playing?
    return libvlc_media_player_get_state(pMediaPlayer_) == libvlc_Stopped;
}
bool VLCWrapper::isStopped()
{
	//  playing?
    return libvlc_media_player_get_state(pMediaPlayer_) == libvlc_Ended;
}

void VLCWrapper::Stop()
{
    // Stop playing
    libvlc_media_player_stop (pMediaPlayer_);
}

int64_t VLCWrapper::GetLength()
{
    int64_t length = libvlc_media_player_get_length(pMediaPlayer_);
    return length;
}

int64_t VLCWrapper::GetTime()
{
    int64_t time = libvlc_media_player_get_time(pMediaPlayer_);    
    return time;
}

void VLCWrapper::SetTime( int64_t newTime )
{
    libvlc_media_player_set_time(pMediaPlayer_,(libvlc_time_t)newTime);
}

void VLCWrapper::Mute( bool mute /*= true*/ )
{
    libvlc_audio_set_mute(pMediaPlayer_, mute);
}

bool VLCWrapper::GetMute()
{
    bool bMuteState=!!libvlc_audio_get_mute(pMediaPlayer_);
    return bMuteState;
}

double VLCWrapper::getVolume()
{
    return (double) libvlc_audio_get_volume(pMediaPlayer_);
}

void VLCWrapper::setVolume( double volume )
{
    libvlc_audio_set_volume(pMediaPlayer_, (int)volume);
}

void VLCWrapper::OpenMedia(const char* pMediaPathName)
{
	// Load a new item
	pMedia_ = libvlc_media_new_path(pVLCInstance_, pMediaPathName);
    libvlc_media_player_set_media (pMediaPlayer_, pMedia_);    
}


void VLCWrapper::loadSubtitle(const char* pSubPathName)
{
	// Load a new item
    libvlc_video_set_subtitle_file (pMediaPlayer_, pSubPathName);    
}

void VLCWrapper::setScale (float ratio)
{
	libvlc_video_set_scale(pMediaPlayer_, ratio);
}
float VLCWrapper::getScale() const
{
	return libvlc_video_get_scale(pMediaPlayer_);
}
void VLCWrapper::setRate (double rate)
{
	libvlc_media_player_set_rate(pMediaPlayer_, rate/100.f);
}
double VLCWrapper::getRate () const
{
	return 100.*libvlc_media_player_get_rate(pMediaPlayer_);
}
void VLCWrapper::setAspect(const char* ratio)
{
	libvlc_video_set_aspect_ratio(pMediaPlayer_, ratio);
}
void VLCWrapper::setAudioDelay(int64_t delay)
{
	libvlc_audio_set_delay(pMediaPlayer_, delay);
}
int64_t VLCWrapper::getAudioDelay()
{
	return libvlc_audio_get_delay(pMediaPlayer_);
}
void VLCWrapper::setSubtitleDelay(int64_t delay)
{
	libvlc_video_set_spu_delay(pMediaPlayer_, delay);
}
int64_t VLCWrapper::getSubtitleDelay()
{
	return libvlc_video_get_spu_delay(pMediaPlayer_);
}
int VLCWrapper::getSubtitlesCount()
{
	return libvlc_video_get_spu_count(pMediaPlayer_);
}
int VLCWrapper::getCurrentSubtitleIndex()
{
	return libvlc_video_get_spu(pMediaPlayer_);
}
void VLCWrapper::setSubtitleIndex(int i)
{
	libvlc_video_set_spu(pMediaPlayer_, i);
}
