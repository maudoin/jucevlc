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
#include <boost/cstdint.hpp>
#include "vlc\libvlc_internal.h"
#include "vlc\plugins\vlc_common.h"
#include "vlc\plugins\vlc_variables.h"
#include "vlc\plugins\vlc_input.h"
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
		    cb->vlcTimeChanged();
            break;
	   case libvlc_MediaPlayerPlaying :
		    cb->vlcStarted();
            break;
	   case libvlc_MediaPlayerPaused:
		    cb->vlcPaused();
            break;
	   case libvlc_MediaPlayerStopped:
		    cb->vlcStopped();
            break;
	} 
}

static void *vlcLock(void *pUserData, void **p_pixels)
{
    DisplayCallback* cb = reinterpret_cast<DisplayCallback*>(pUserData); 
	return cb?cb->vlcLock( p_pixels):NULL;
}

static void vlcUnlock(void *pUserData, void *id, void *const *p_pixels)
{
    DisplayCallback* cb = reinterpret_cast<DisplayCallback*>(pUserData); 
	if(cb)
		cb->vlcUnlock( id, p_pixels);
}

static void vlcDisplay(void *pUserData, void *id)
{
    DisplayCallback* cb = reinterpret_cast<DisplayCallback*>(pUserData); 
	if(cb)
		cb->vlcDisplay(id);
}

static int popupCallback(vlc_object_t *p_this, const char *psz_variable,
                        vlc_value_t old_val, vlc_value_t new_val, void *param )
{
    VLC_UNUSED( p_this ); VLC_UNUSED( psz_variable ); VLC_UNUSED( old_val );

    InputCallBack* cb = reinterpret_cast<InputCallBack*>(param); 
	if(cb)
		cb->vlcPopupCallback(new_val.b_bool);
    return VLC_SUCCESS;
}

static int fullscreenControlCallback(vlc_object_t *p_this, const char *psz_variable,
                        vlc_value_t old_val, vlc_value_t new_val, void *param )
{
    VLC_UNUSED( p_this ); VLC_UNUSED( psz_variable ); VLC_UNUSED( old_val );

    InputCallBack* cb = reinterpret_cast<InputCallBack*>(param); 
	if(cb)
		cb->vlcFullScreenControlCallback();
    return VLC_SUCCESS;
}
static int onMouseMoveCallback(vlc_object_t *p_vout, const char *psz_var, vlc_value_t old,
                        vlc_value_t val, void *p_data)
{
    VLC_UNUSED(old);

    MouseInputCallBack* cb = reinterpret_cast<MouseInputCallBack*>(p_data); 
	if(cb)
	{
		cb->vlcMouseMove(val.coords.x, val.coords.y, var_GetInteger( p_vout, "mouse-button-down" ));
	}
    return VLC_SUCCESS;
}
static int onMouseClickCallback(vlc_object_t *p_vout, const char *psz_var, vlc_value_t old,
                        vlc_value_t val, void *p_data)
{
    VLC_UNUSED(old);
    VLC_UNUSED(p_vout);

    MouseInputCallBack* cb = reinterpret_cast<MouseInputCallBack*>(p_data); 
	if(cb)
	{
		cb->vlcMouseClick(val.coords.x, val.coords.y, var_GetInteger( p_vout, "mouse-button-down" ));
	}
    return VLC_SUCCESS;
}
VLCWrapper::VLCWrapper(void)
:	pVLCInstance_(0),
	pMediaPlayer_(0),
	pMedia_(0),
    pEventManager_(0)
{
	static const char * const vlc_args[] = {
		"-I", "dumy"      // No special interface
		,"--ignore-config" // Don't use VLC's config
        ,"--no-video-title-show"
		,"--plugin-path=./plugins"
		//,"--video-filter=crop"
	};

	// init vlc modules, should be done only once
	//pVLCInstance_ = libvlc_new (sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
	pVLCInstance_ = libvlc_new (0, NULL);
     
    // Create a media player playing environement
    pMediaPlayer_ = libvlc_media_player_new(pVLCInstance_);

    // Create an event manager for the player for handling e.g. time change events
    pEventManager_ = libvlc_media_player_event_manager(pMediaPlayer_);

	libvlc_video_set_mouse_input(pMediaPlayer_, true);
	libvlc_video_set_key_input(pMediaPlayer_, true);
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
	    libvlc_video_set_callbacks(pMediaPlayer_, vlcLock, vlcUnlock, vlcDisplay, cb);
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

void VLCWrapper::setScale (double ratio)
{
	libvlc_video_set_scale(pMediaPlayer_, ratio/100.f);
}
double VLCWrapper::getScale() const
{
	return libvlc_video_get_scale(pMediaPlayer_)*100.;
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

void VLCWrapper::SetInputCallBack(InputCallBack* cb)
{
    //var_Create( p_playlist, "fullscreen", VLC_VAR_BOOL | VLC_VAR_DOINHERIT );
    //var_Create( p_playlist, "video-on-top", VLC_VAR_BOOL | VLC_VAR_DOINHERIT );

	vlc_object_t *obj = VLC_OBJECT(pVLCInstance_->p_libvlc_int);
//	var_DelCallback( obj, "intf-popupmenu", cb?popupCallback:0, cb );
//	var_DelCallback( obj, "intf-toggle-fscontrol", cb?fullscreenControlCallback:0, cb );
	var_AddCallback( obj, "intf-popupmenu", cb?popupCallback:0, cb );
	var_AddCallback( obj, "intf-toggle-fscontrol", cb?fullscreenControlCallback:0, cb );

}

#include <assert.h>
#include "vlc/media_player_internal.h"
#include "vlc/plugins/vlc_threads.h"


static inline void lock_input(libvlc_media_player_t *mp)
{
    vlc_mutex_lock(&mp->input.lock);
}

static inline void unlock_input(libvlc_media_player_t *mp)
{
    vlc_mutex_unlock(&mp->input.lock);
}
/*
 * Retrieve the input thread. Be sure to release the object
 * once you are done with it. (libvlc Internal)
 */
input_thread_t *libvlc_get_input_thread( libvlc_media_player_t *p_mi )
{
    input_thread_t *p_input_thread;

    assert( p_mi );

    lock_input(p_mi);
    p_input_thread = p_mi->input.p_thread;
    if( p_input_thread )
        vlc_object_hold( p_input_thread );
    else
        libvlc_printerr( "No active input" );
    unlock_input(p_mi);

    return p_input_thread;
}
/*
 * Remember to release the returned vout_thread_t.
 */
static vout_thread_t **GetVouts( libvlc_media_player_t *p_mi, size_t *n )
{
    input_thread_t *p_input = libvlc_get_input_thread( p_mi );
    if( !p_input )
    {
        *n = 0;
        return NULL;
    }

    vout_thread_t **pp_vouts;
    if (input_Control( p_input, INPUT_GET_VOUTS, &pp_vouts, n))
    {
        *n = 0;
        pp_vouts = NULL;
    }
    vlc_object_release (p_input);
    return pp_vouts;
}

static vout_thread_t *GetVout (libvlc_media_player_t *mp, size_t num)
{
    vout_thread_t *p_vout = NULL;
    size_t n;
    vout_thread_t **pp_vouts = GetVouts (mp, &n);
    if (pp_vouts == NULL)
        goto err;

    if (num < n)
        p_vout = pp_vouts[num];

    for (size_t i = 0; i < n; i++)
        if (i != num)
            vlc_object_release (pp_vouts[i]);
    //free (pp_vouts);

    if (p_vout == NULL)
err:
        libvlc_printerr ("Video output not active");
    return p_vout;
}


bool VLCWrapper::setMouseInputCallBack(MouseInputCallBack* cb)
{

	//var_Create (obj, "key-pressed", VLC_VAR_INTEGER);
    vout_thread_t *p_vout = GetVout (pMediaPlayer_, 0);
	if(p_vout)
	{
		var_AddCallback( p_vout, "mouse-moved", cb?onMouseMoveCallback:0, cb );
		var_AddCallback( p_vout, "mouse-clicked", cb?onMouseClickCallback:0, cb );
	
		vlc_object_release (p_vout);
	}
	return p_vout != 0;
}


	
std::vector<std::string> VLCWrapper::getCropList()
{
	std::vector<std::string> list;
    vout_thread_t *p_vout = GetVout (pMediaPlayer_, 0);
	if(p_vout)
	{
        vlc_value_t val_list, text_list;
		var_Change( p_vout, "crop", VLC_VAR_GETLIST,
									&val_list, &text_list );
        for( int i = 0; i < val_list.p_list->i_count; i++ )
        {
			list.push_back( val_list.p_list->p_values[i].psz_string );
        }
        var_FreeList( &val_list, &text_list );

		vlc_object_release (p_vout);
	}
	return list;

}
void VLCWrapper::setCrop(std::string const& ratio)
{
    vout_thread_t *p_vout = GetVout (pMediaPlayer_, 0);
	if(p_vout)
	{
        var_SetString( p_vout, "crop",ratio.c_str());

		vlc_object_release (p_vout);
	}
}

std::string VLCWrapper::getCrop()
{
	std::string crop;
    vout_thread_t *p_vout = GetVout (pMediaPlayer_, 0);
	if(p_vout)
	{
		char* str = var_GetString( p_vout, "crop" );
		crop = str;
		//free( str );

		vlc_object_release (p_vout);
	}
	return crop;
}
void VLCWrapper::setAutoCrop(bool autoCrop)
{
    vout_thread_t *p_vout = GetVout (pMediaPlayer_, 0);
	if(p_vout)
	{
        var_SetString( p_vout, "video-filter", "crop" );
        var_SetBool( p_vout, "autocrop",autoCrop);

		vlc_object_release (p_vout);
	}
}

bool VLCWrapper::isAutoCrop()
{
	bool autoCrop = false;
    vout_thread_t *p_vout = GetVout (pMediaPlayer_, 0);
	if(p_vout)
	{
		autoCrop = var_GetBool( p_vout, "autocrop" );
		vlc_object_release (p_vout);
	}
	return autoCrop;
}


std::vector< std::pair<int, std::string> > VLCWrapper::getVideoTrackList()
{
	std::vector< std::pair<int, std::string> > list;
	libvlc_track_description_t *desc =libvlc_video_get_track_description(pMediaPlayer_);
	for(libvlc_track_description_t *it=desc;it;it=it->p_next)
	{
		list.push_back(std::pair<int, std::string>(it->i_id, it->psz_name));
	}
	libvlc_free(desc);
	return list;
}
void VLCWrapper::setVideoTrack(int n)
{
	libvlc_video_set_track(pMediaPlayer_, n);
}
int VLCWrapper::getVideoTrack()
{
	return libvlc_video_get_track(pMediaPlayer_);
}

std::vector< std::pair<int, std::string> > VLCWrapper::getAudioTrackList()
{
	std::vector< std::pair<int, std::string> > list;
	libvlc_track_description_t *desc =libvlc_audio_get_track_description(pMediaPlayer_);
	for(libvlc_track_description_t *it=desc;it;it=it->p_next)
	{
		list.push_back(std::pair<int, std::string>(it->i_id, it->psz_name));
	}
	libvlc_free(desc);
	return list;
}

void VLCWrapper::setAudioTrack(int n)
{
	libvlc_audio_set_track(pMediaPlayer_, n);
}

int VLCWrapper::getAudioTrack()
{
	return libvlc_audio_get_track(pMediaPlayer_);
}
