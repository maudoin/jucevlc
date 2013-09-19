// VLCWrapper/Alex Skoruppa (alex.skoruppa@googlemail.com)
// http://www.codeproject.com/info/cpol10.aspx

#include "VLCWrapper.h"
#include "vlc\vlc.h"
#include "vlc\libvlc_events.h"
#include <boost/cstdint.hpp>
#include "vlc\libvlc_internal.h"
#include "vlc\plugins\vlc_common.h"
#include "vlc\plugins\vlc_variables.h"
#include "vlc\plugins\vlc_input.h"
#include <stdio.h>
#include <sstream>
#include <deque>
#include <algorithm>
#include <iterator>

#include <assert.h>
#include "vlc/media_player_internal.h"
#include "vlc/plugins/vlc_threads.h"
#include "vlc/plugins/vlc_aout_intf.h"

class VLCInstancePtr
{
	libvlc_instance_t*ptr;
public:
	VLCInstancePtr():ptr(0){}
	void mayInit( int argc , const char *const *argv )
	{
		if(ptr==0)
		{
			ptr = libvlc_new (0, NULL);
		}
	}
	~VLCInstancePtr(){libvlc_release(ptr);}
	libvlc_instance_t* get(){return ptr;}
	const libvlc_instance_t* get()const{return ptr;}
};

VLCInstancePtr pVLCInstance_;        ///< The VLC instance.

//----------------------------------- HELPERS
class MediaListScopedLock
{
	libvlc_media_list_t* m_ml;
public:
	MediaListScopedLock( libvlc_media_list_t* ml):m_ml(ml)
	{
		if(m_ml)libvlc_media_list_lock(m_ml);
	}
	~MediaListScopedLock()
	{
		if(m_ml)libvlc_media_list_unlock(m_ml);
	}
};
//-----------------------------------

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
	   case libvlc_MediaPlayerEndReached:
		    cb->vlcStopped();
            break;
	} 
}

//-----------------------------------

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

VLCUPNPMediaList::VLCUPNPMediaList()
{
	pVLCInstance_.mayInit(0, NULL);
	//media_discoverer
    pMediaDiscoverer_ = libvlc_media_discoverer_new_from_name(pVLCInstance_.get(),"upnp");

	pUPNPMediaList_ = libvlc_media_discoverer_media_list(pMediaDiscoverer_);

}
VLCUPNPMediaList::~VLCUPNPMediaList()
{
	libvlc_media_list_release(pUPNPMediaList_) ;
	libvlc_media_discoverer_release( pMediaDiscoverer_);

}

VLCWrapper::VLCWrapper(void)
:	pMediaPlayer_(0),
    pEventManager_(0),
	m_videoAdjustEnabled(0)
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
	pVLCInstance_.mayInit(0, NULL);
     
    // Create a media player playing environement
	pMediaPlayer_ = libvlc_media_player_new(pVLCInstance_.get());
	

	//list player
    mlp = libvlc_media_list_player_new(pVLCInstance_.get());
 
	//media list
    ml = libvlc_media_list_new(pVLCInstance_.get());

    /* Use our media list */
    libvlc_media_list_player_set_media_list(mlp, ml);

    /* Use a given media player */
    libvlc_media_list_player_set_media_player(mlp, pMediaPlayer_);



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
    libvlc_media_list_player_release (mlp);
    libvlc_media_list_release (ml);
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

void VLCWrapper::play()
{
	// play the media_player
    libvlc_media_list_player_play (mlp);
}

void VLCWrapper::Pause()
{
	// Pause playing
    libvlc_media_list_player_pause (mlp);
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
    return libvlc_media_list_player_get_state(mlp) == libvlc_Paused;
}
bool VLCWrapper::isPlaying()
{
    return libvlc_media_list_player_get_state(mlp) == libvlc_Playing;
}

bool VLCWrapper::isStopping()
{
	//  playing?
    return libvlc_media_list_player_get_state(mlp) == libvlc_Stopped;
}
bool VLCWrapper::isStopped()
{
	//  playing?
    return libvlc_media_list_player_get_state(mlp) == libvlc_Ended;
}

void VLCWrapper::Stop()
{
    // Stop playing
    libvlc_media_list_player_stop (mlp);
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
std::string VLCWrapper::getAspect() const
{
	char* aspect = libvlc_video_get_aspect_ratio(pMediaPlayer_);
	return std::string(aspect?aspect:"");
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

	vlc_object_t *obj = VLC_OBJECT(pVLCInstance_.get()->p_libvlc_int);
//	var_DelCallback( obj, "intf-popupmenu", cb?popupCallback:0, cb );
//	var_DelCallback( obj, "intf-toggle-fscontrol", cb?fullscreenControlCallback:0, cb );
	var_AddCallback( obj, "intf-popupmenu", cb?popupCallback:0, cb );
	var_AddCallback( obj, "intf-toggle-fscontrol", cb?fullscreenControlCallback:0, cb );

}



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

/*
 * Remember to release the returned audio_output_t since it is locked at
 * the end of this function.
 */
static audio_output_t *GetAOut( libvlc_media_player_t *mp )
{
    assert( mp != NULL );

    input_thread_t *p_input = libvlc_get_input_thread( mp );
    if( p_input == NULL )
        return NULL;

    audio_output_t * p_aout = input_GetAout( p_input );
    vlc_object_release( p_input );
    if( p_aout == NULL )
        libvlc_printerr( "No active audio output" );
    return p_aout;
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
    libvlc_free (pp_vouts);

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
		if(str)
		{
			crop = str;
			libvlc_free( str );
		}

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

void VLCWrapper::setVoutOptionInt(const char* name, int v)
{
    vout_thread_t *p_vout = GetVout (pMediaPlayer_, 0);
	if(p_vout)
	{
		var_SetInteger( p_vout, name,v);

		vlc_object_release (p_vout);
	}
}

int VLCWrapper::getVoutOptionInt(const char* name)
{
	int v = false;
    vout_thread_t *p_vout = GetVout (pMediaPlayer_, 0);
	if(p_vout)
	{
		v = var_GetInteger( p_vout, name );
		vlc_object_release (p_vout);
	}
	return v;
}

/* Return the order in which filters should be inserted */
static int FilterOrder( const char *psz_name )
{
    static const struct {
        const char *psz_name;
        int        i_order;
    } filter[] = {
        { "equalizer",  0 },
        { NULL,         INT_MAX },
    };
    for( int i = 0; filter[i].psz_name; i++ )
    {
        if( !strcmp( filter[i].psz_name, psz_name ) )
            return filter[i].i_order;
    }
    return INT_MAX;
}
void VLCWrapper::setAoutFilterOptionString(const char* name, std::string const& filter, std::string const& v)
{
	audio_output_t *p_aout = GetAOut (pMediaPlayer_);
	if(p_aout)
	{
		aout_EnableFilter( pMediaPlayer_, filter.c_str(), !v.empty() );
		var_SetString( p_aout, name,v.c_str());

		vlc_object_release (p_aout);
	}
	else
	{
		std::string audioFilters = getConfigOptionString("audio-filter");
		if(std::string::npos == audioFilters.find(AOUT_FILTER_EQUALIZER))
		{
			//set if missing
			if(!audioFilters.empty())
			{
				//append if not alone
				audioFilters += ":";
			}
			audioFilters += AOUT_FILTER_EQUALIZER;
			setConfigOptionString("audio-filter", audioFilters);
		}

		setConfigOptionString(name, v);
	}
}

std::string VLCWrapper::getAoutFilterOptionString(const char* name)
{
	std::string v;
	audio_output_t *p_aout = GetAOut (pMediaPlayer_);
	if(p_aout)
	{
		char* s = var_GetString( p_aout, name );
		if(s)
		{
			v = s;
			libvlc_free(s);
		}
		vlc_object_release (p_aout);
	}
	else
	{
		return getConfigOptionString(name);
	}
	return v;
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
  
void VLCWrapper::setVideoAdjust(bool n)
{
	libvlc_video_set_adjust_int(pMediaPlayer_, libvlc_adjust_Enable, n?1:0);
	m_videoAdjustEnabled = n;
}
bool VLCWrapper::getVideoAdjust()
{
	return m_videoAdjustEnabled;
}
void VLCWrapper::setVideoContrast(double n)
{
	libvlc_video_set_adjust_float(pMediaPlayer_, libvlc_adjust_Contrast, (float)n);
}
double VLCWrapper::getVideoContrast()
{
	return (double)libvlc_video_get_adjust_float(pMediaPlayer_, libvlc_adjust_Contrast);
}
void VLCWrapper::setVideoBrightness(double n)
{
	libvlc_video_set_adjust_float(pMediaPlayer_, libvlc_adjust_Brightness, (float)n);
}
double VLCWrapper::getVideoBrightness()
{
	return (double)libvlc_video_get_adjust_float(pMediaPlayer_, libvlc_adjust_Brightness);
}

void VLCWrapper::setVideoHue(double n)
{
	libvlc_video_set_adjust_int(pMediaPlayer_, libvlc_adjust_Hue, (int)n);
}
double VLCWrapper::getVideoHue()
{
	return (double)libvlc_video_get_adjust_int(pMediaPlayer_, libvlc_adjust_Hue);
}
void VLCWrapper::setVideoSaturation(double n)
{
	libvlc_video_set_adjust_float(pMediaPlayer_, libvlc_adjust_Saturation, (float)n);
}
double VLCWrapper::getVideoSaturation()
{
	return (double)libvlc_video_get_adjust_float(pMediaPlayer_, libvlc_adjust_Saturation);
}
void VLCWrapper::setVideoGamma(double n)
{
	libvlc_video_set_adjust_float(pMediaPlayer_, libvlc_adjust_Gamma, (float)n);
}
double VLCWrapper::getVideoGamma()
{
	return (double)libvlc_video_get_adjust_float(pMediaPlayer_, libvlc_adjust_Gamma);
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

bool addMediaSubItemsToMediaList(libvlc_media_t* media, libvlc_media_list_t* ml, int index)
{
	
	libvlc_media_list_t* subMediaList = libvlc_media_subitems(media);
	if(subMediaList)
	{
		{
			MediaListScopedLock lock(subMediaList);

			//add subitems individually
			int jmax = libvlc_media_list_count(subMediaList);
			for(int j=0;j<jmax;++j)
			{
				libvlc_media_t* media = libvlc_media_list_item_at_index(subMediaList, j);
				libvlc_media_list_insert_media(ml, media, index+j);
			}
		}
		libvlc_media_list_release(subMediaList) ;
		return true;
	}
	return false;
}
std::string urlDecode(std::string const &SRC) {
    std::string ret;
    char ch;
    std::string::size_type i, ii;
    for (i=0; i<SRC.length(); i++) {
        if (int(SRC[i])==37) {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=SRC[i];
        }
    }
    return (ret);
}
std::string getMediaName(libvlc_media_t* media)
{
	char* desc = libvlc_media_get_meta 	( media, libvlc_meta_Title ) ;	
	std::string name = (desc?desc:"???");
	libvlc_free(desc);
	return name;
	/*
	char* str = libvlc_media_get_mrl(media );
	std::string url = str?urlDecode(str):"";
	libvlc_free(str);
	if(url.find("rar")==0)
	{
		//strip content filename (after pipe character) for rars
		std::string::size_type i = url.find_last_of("|");
		if(i != std::string::npos)
		{
			url = url.substr(0, i);
		}

	}
		
	std::string::size_type i = url.find_last_of("/\\");
	return i == std::string::npos ? url : url.substr(i+1);
	*/
}
std::vector<std::string> VLCWrapper::getCurrentPlayList()
{	
	std::vector<std::string> out;
	MediaListScopedLock lock(ml);
	int max = libvlc_media_list_count(ml);
	for(int i=0;i<max;++i)
	{
		libvlc_media_t* media = libvlc_media_list_item_at_index(ml, i);

		if(addMediaSubItemsToMediaList(media, ml, i+1))
		{
			//clear multi media item and restart
			libvlc_media_list_remove_index(ml, i);
			//reset counters
			max = libvlc_media_list_count(ml);
			i--;
			continue;
		}
		libvlc_media_parse(media);
		out.push_back(getMediaName(media));
		
		libvlc_media_release(media);
	}
	return out;
}
void VLCWrapper::removePlaylistItem(int index)
{
	libvlc_media_list_remove_index(ml, index);
}
void VLCWrapper::clearPlayList()
{
	while(	libvlc_media_list_count(ml)> 0)
	{
		removePlaylistItem(0);
	}
}
int VLCWrapper::addPlayListItem(std::string const& path)
{
	libvlc_media_t* pMedia = libvlc_media_new_path(pVLCInstance_.get(), path.c_str());
	libvlc_media_list_add_media(ml, pMedia);
	return libvlc_media_list_count(ml) -1;
}
std::string VLCWrapper::getCurrentPlayListItem()
{
	libvlc_media_t* media = libvlc_media_player_get_media(pMediaPlayer_);
	if(!media)
	{
		return "";
	}
	libvlc_media_parse(media);
	return getMediaName(media);
}

int VLCWrapper::getCurrentPlayListItemIndex()
{
	libvlc_media_t* media = libvlc_media_player_get_media(pMediaPlayer_);
	if(!media)
	{
		return -1;
	}
	return libvlc_media_list_index_of_item(ml, media );
}

void VLCWrapper::playPlayListItem(int index)
{
	libvlc_media_list_player_play_item_at_index(mlp, index);
}

int VLCWrapper::getConfigOptionInt(const char* name) const
{
	return config_GetInt(pVLCInstance_.get(), name);
}
void VLCWrapper::setConfigOptionInt(const char* name, int value)
{
	config_PutInt(pVLCInstance_.get(), name, value);
}

bool VLCWrapper::getConfigOptionBool(const char* name)const
{
	return (bool)(config_GetInt(pVLCInstance_.get(), name)!=0);
}

void VLCWrapper::setConfigOptionBool(const char* name, bool value)
{
	config_PutInt(pVLCInstance_.get(), name, value);
}

std::pair<int, std::vector<std::pair<int, std::string> >> VLCWrapper::getConfigOptionInfoInt(const char* name)const
{
    module_config_t *p_module_config = config_FindConfig( (vlc_object_t*)pVLCInstance_.get(), name );
	
    if( p_module_config->pf_update_list )
    {
       vlc_value_t val;
       val.i_int = p_module_config->value.i;

       p_module_config->pf_update_list((vlc_object_t*)pVLCInstance_.get(), p_module_config->psz_name, val, val, NULL);

       // assume in any case that dirty was set to true
       // because lazy programmes will use the same callback for
       // this, like the one behind the refresh push button?
       p_module_config->b_dirty = false;
    }

	std::pair<int,std::vector<std::pair<int, std::string> >> info;
	if(p_module_config->pi_list)
	{
		for( int i_index = 0; i_index < p_module_config->i_list; i_index++ )
		{
			info.second.push_back(std::pair<int, std::string>(p_module_config->pi_list[i_index], p_module_config->ppsz_list_text[i_index]));
		}
	}
	else
	{
		for( int i_index = p_module_config->min.i; i_index <= p_module_config->max.i; i_index++ )
		{
			std::ostringstream oss;
			oss<<i_index;
			info.second.push_back(std::pair<int, std::string>(i_index, oss.str()));
		}
	}
	info.first = p_module_config->value.i;

	return info;
}


std::string VLCWrapper::getConfigOptionString(const char* name)const
{
	return config_GetPsz(pVLCInstance_.get(), name);
}

void VLCWrapper::setConfigOptionString(const char* name, std::string const& value)
{
	config_PutPsz(pVLCInstance_.get(), name, value.c_str());
}
std::pair<std::string, std::vector<std::pair<std::string, std::string> >> VLCWrapper::getConfigOptionInfoString(const char* name)const
{
    module_config_t *p_module_config = config_FindConfig( (vlc_object_t*)pVLCInstance_.get(), name );
	
    if( p_module_config->pf_update_list )
    {
        vlc_value_t val;
        val.psz_string = strdup(p_module_config->value.psz);

        p_module_config->pf_update_list((vlc_object_t*)pVLCInstance_.get(), p_module_config->psz_name, val, val, NULL);

        // assume in any case that dirty was set to true
        // because lazy programmes will use the same callback for
        // this, like the one behind the refresh push button?
        p_module_config->b_dirty = false;

        free( val.psz_string );
    }

	std::pair<std::string,std::vector<std::pair<std::string, std::string> >> info;
	if(p_module_config->ppsz_list)
	{
		for( int i_index = 0; i_index < p_module_config->i_list; i_index++ )
		{
			if( !p_module_config->ppsz_list[i_index] )
			{
			
				info.second.push_back(std::pair<std::string, std::string>("", ""));
				  continue;
			}

			info.second.push_back(std::pair<std::string, std::string>(p_module_config->ppsz_list[i_index], 
				(p_module_config->ppsz_list_text &&
									p_module_config->ppsz_list_text[i_index])?
									p_module_config->ppsz_list_text[i_index] :
									p_module_config->ppsz_list[i_index]));
		}
	}
	info.first = p_module_config->value.psz;

	return info;
}

//include the dot
std::string getExtension(std::string const& path)
{
	std::string::size_type p = path.find_last_of(".");
	if(p == std::string::npos)
	{
		return "";
	}
	return path.substr(p);
}

void readMediaList(libvlc_media_list_t* mediaList, std::vector<std::pair<std::string, std::string> >& list, std::string const& prefix = "")
{
	if(mediaList)
	{
		MediaListScopedLock lock(mediaList);

		//add subitems individually
		int jmax = libvlc_media_list_count(mediaList);
		for(int j=0;j<jmax;++j)
		{
			libvlc_media_t* media = libvlc_media_list_item_at_index(mediaList, j);
				
			libvlc_media_parse(media);
			
			char* desc = libvlc_media_get_meta 	( media, libvlc_meta_Title ) ;	
			std::string name = (desc?desc:"???");
			if(!prefix.empty())
			{
				name = prefix + "/" + name;
			}
			libvlc_free(desc);
				
			libvlc_media_list_t* subMediaList = libvlc_media_subitems(media);
			if(subMediaList)
			{
				//recurse
				readMediaList(subMediaList, list, name);

				libvlc_media_list_release(subMediaList);
			}
			else
			{
				char* mrl = libvlc_media_get_mrl 	( media ) ;	
				if(std::string::npos == std::string(mrl).find("vlc://nop"))
				{
					//real media, keep it
					list.push_back(std::pair<std::string, std::string>(name+getExtension(mrl), mrl));
				}
				libvlc_free(mrl);
			}
			
			libvlc_media_release(media);
		}
	}
}

std::vector<std::pair<std::string, std::string> > VLCUPNPMediaList::getUPNPList(std::vector<std::string> const& path)
{
	std::vector<std::pair<std::string, std::string> > list;
	//readMediaList(pUPNPMediaList_, list);

	std::deque<std::string> nameList;
	std::copy(path.begin(), path.end(), std::back_inserter(nameList));

	libvlc_media_list_t* nextMedialist = pUPNPMediaList_;
	libvlc_media_list_retain(pUPNPMediaList_);//media lists are released

	while(nextMedialist)
	{
		libvlc_media_list_t* currentMediaList = nextMedialist;
		nextMedialist = 0;
		MediaListScopedLock lock(currentMediaList);

		//add subitems individually
		int jmax = libvlc_media_list_count(currentMediaList);
		for(int j=0;j<jmax;++j)
		{
			libvlc_media_t* media = libvlc_media_list_item_at_index(currentMediaList, j);
				
			libvlc_media_parse(media);
			
			char* desc = libvlc_media_get_meta 	( media, libvlc_meta_Title ) ;	
			std::string name = (desc?desc:"???");
			libvlc_free(desc);

			if(nameList.empty())
			{
				//list
				char* mrl = libvlc_media_get_mrl 	( media ) ;	
				list.push_back(std::pair<std::string, std::string>(name+getExtension(mrl), mrl));
				libvlc_free(mrl);
			}
			else if(name == nameList.front())
			{
				//found path entry, enter next entry
				nameList.pop_front();

				nextMedialist = libvlc_media_subitems(media);

				
				//look no further in the current entry
				jmax = 0;
			}
				
			
			libvlc_media_release(media);
		}
		
		libvlc_media_list_release(currentMediaList);
	}
	return list;
}