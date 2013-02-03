/************************************************************************
    This file is part of VLCWrapper.
    
    File:   VLCWrapperImpl.h
    Desc.:  Private Implementation of VLCWrapper.

	Author:  Alex Skoruppa
	Date:    08/10/2009
	Updated: 03/12/2012
	eM@il:   alex.skoruppa@googlemail.com

	VLCWrapper is distributed under the Code Project Open License (CPOL).

	You should have received a copy of the Code Project Open License
	along with VLCWrapper.  If not, see <http://www.codeproject.com/info/cpol10.aspx>.
************************************************************************/
#ifndef __VLCWRAPPERIMPL_H__
#define __VLCWRAPPERIMPL_H__

#include "VLCWrapper.h"

struct libvlc_instance_t;
struct libvlc_media_player_t;
struct libvlc_media_t;
struct libvlc_event_manager_t;

class VLCWrapperImpl
{
    libvlc_instance_t*       pVLCInstance_;        ///< The VLC instance.
	libvlc_media_player_t*   pMediaPlayer_;        ///< The VLC media player object.
	libvlc_media_t*          pMedia_;              ///< The media played by the media player.
    libvlc_event_manager_t*  pEventManager_;       ///< The event manger for the loaded media file.    
    VLCEventHandler          eventHandler;         ///< An event handler for the media player.    

public:
	VLCWrapperImpl(void);
	~VLCWrapperImpl(void);
    void SetOutputWindow(void* pHwnd);    
    void SetDisplayCallback(DisplayCallback* cb);
    void SetCallbacks()
	{
		//libvlc_audio_set_callbacks
		//libvlc_video_set_callbacks(pMediaPlayer_, lock, unlock, display, &ctx);
		//libvlc_video_set_format(pMediaPlayer_, "RV16", VIDEOWIDTH, VIDEOHEIGHT, VIDEOWIDTH*2);
	}
	void SetEventHandler(VLCEventHandler event, void* pUserData); 
    void Play();
    void Pause();
    void Stop();
    int64_t GetLength();
    int64_t GetTime();
	void SetTime(int64_t newTime);
	void Mute(bool mute = true);
    bool GetMute();
    int  GetVolume();
	void SetVolume(int volume);
    void OpenMedia(const char* pMediaPathName);
};

#endif // __VLCWRAPPERIMPL_H__