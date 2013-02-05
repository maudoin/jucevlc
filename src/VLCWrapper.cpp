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

static void *lock(void *data, void **p_pixels)
{
	return data?((DisplayCallback*)data)->lock( p_pixels):NULL;
}

static void unlock(void *data, void *id, void *const *p_pixels)
{
	if(data)
		((DisplayCallback*)data)->unlock( id, p_pixels);
}

static void display(void *data, void *id)
{
	if(data)
		((DisplayCallback*)data)->display(id);
}

VLCWrapper::VLCWrapper(void)
:	pVLCInstance_(0),
	pMediaPlayer_(0),
	pMedia_(0),
    pEventManager_(0),
    eventHandler(0)
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

VLCWrapper::~VLCWrapper(void)
{
    // Free the media_player
    libvlc_media_player_release (pMediaPlayer_);
	libvlc_release (pVLCInstance_);
}

void VLCWrapper::SetDisplayCallback(DisplayCallback* cb)
{
	if(cb)
	{
	    libvlc_video_set_callbacks(pMediaPlayer_, lock, unlock, display, cb);
	    libvlc_video_set_format(pMediaPlayer_, "RV24", cb->imageWidth(), cb->imageHeight(), cb->imageStride());
	}
}
void VLCWrapper::SetOutputWindow(void* pHwnd)
{
    // Set the output window    
	libvlc_media_player_set_hwnd(pMediaPlayer_, pHwnd);
}

void VLCWrapper::SetEventHandler( VLCEventHandler event, void* pUserData )
{
    eventHandler = event;
    libvlc_event_attach(pEventManager_,        
                        libvlc_MediaPlayerTimeChanged,
                        eventHandler,
                        pUserData);
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

int VLCWrapper::GetVolume()
{
    int volume = libvlc_audio_get_volume(pMediaPlayer_);
    return volume;
}

void VLCWrapper::SetVolume( int volume )
{
    libvlc_audio_set_volume(pMediaPlayer_, volume);
}

void VLCWrapper::OpenMedia(const char* pMediaPathName)
{
	// Load a new item
	pMedia_ = libvlc_media_new_path(pVLCInstance_, pMediaPathName);
    libvlc_media_player_set_media (pMediaPlayer_, pMedia_);    
}
