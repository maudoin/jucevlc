/************************************************************************
    This file is part of VLCWrapper.
    
    File:    VLCWrapper.h
    Desc.:   An simple C++-interface to libvlc.

	Author:  Alex Skoruppa
	Date:    08/10/2009
	Updated: 03/12/2012
	eM@il:   alex.skoruppa@googlemail.com

	VLCWrapper is distributed under the Code Project Open License (CPOL).

	You should have received a copy of the Code Project Open License
	along with VLCWrapper.  If not, see <http://www.codeproject.com/info/cpol10.aspx>.
************************************************************************/
#ifndef __VLCWRAPPER_H__
#define __VLCWRAPPER_H__

#include <memory>
#include <string>

struct libvlc_instance_t;
struct libvlc_media_player_t;
struct libvlc_media_t;
struct libvlc_event_manager_t;

// Typedefs for old MS compilers
typedef unsigned __int32	uint32_t;
typedef __int64             int64_t;


class DisplayCallback
{
public:
	virtual void *lock(void **p_pixels) = 0;

	virtual void unlock(void *id, void *const *p_pixels) = 0;

	virtual void display(void *id) = 0;
	
};
class EventCallBack
{
public:
	virtual void timeChanged() = 0;
	virtual void paused() = 0;
	virtual void started() = 0;
	virtual void stopped() = 0;
};
class VLCWrapper
{
    libvlc_instance_t*       pVLCInstance_;        ///< The VLC instance.
	libvlc_media_player_t*   pMediaPlayer_;        ///< The VLC media player object.
	libvlc_media_t*          pMedia_;              ///< The media played by the media player.
    libvlc_event_manager_t*  pEventManager_;       ///< The event manger for the loaded media file.    
public:
	VLCWrapper(void);
	~VLCWrapper(void);

    /** Set window for media output.
    *   @param [in] pHwnd window, on Windows a HWND handle. */
    void SetOutputWindow(void* pHwnd);
    void SetDisplayCallback(DisplayCallback* cb);
    void SetEventCallBack(EventCallBack* cb);
    void SetBufferFormat(int imageWidth, int imageHeight, int imageStride);




    /** Open a media file.
    *   @param [in] pMediaPathName PathName of the media file. */
    void OpenMedia(const char* pMediaPathName);

    /** Start playback. */
    void Play();

	bool isPaused();

    /** Pause playback. */
    void Pause();
    
    /** Stop playback. */
    void Stop();

    /** Get length of media in milliseconds. Call this in the event handler,
    *   otherwise the result is not reliable!!!
    *   @return The media length in milliseconds. */
    int64_t GetLength();

    /** Get actual position of media in milliseconds. Call this in the event handler,
    *   otherwise the result is not reliable!!!
    *   @return The media position in milliseconds. */
    int64_t GetTime();

    /** Set new position of media in milliseconds.
    *   @param [in] newTime The new media position in milliseconds. */
    void SetTime(int64_t newTime);

    /** Mutes the audio output of playback.
    *   @param [in] mute True or false. */
    void Mute(bool mute = true);

    /** Get mute state of playback.
    *   @return True or false. */
    bool GetMute();

    /** Returns the actual audio volume.
    *   @return The actual audio volume. */
    int  GetVolume();

    /** Set the actual audio volume.
    *   @param [in] volume New volume level. */
    void SetVolume(int volume);    

	
	std::string getInfo() const;
};

#endif // __VLCWRAPPER_H__