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
#include <vector>

#define CONFIG_BOOL_OPTION_HARDWARE "ffmpeg-hw" 
#define CONFIG_INT_OPTION_SUBTITLE_SIZE "freetype-rel-fontsize" //int list
#define CONFIG_INT_OPTION_SUBTITLE_OUTLINE_THICKNESS "freetype-outline-thickness" //int list
#define CONFIG_INT_OPTION_VIDEO_QUALITY "postproc-q" 
#define CONFIG_INT_OPTION_VIDEO_DEINTERLACE "deinterlace" 
#define CONFIG_STRING_OPTION_VIDEO_DEINTERLACE_MODE "deinterlace-mode"
#define CONFIG_COLOR_OPTION_SUBTITLE_COLOR "freetype-color"
#define CONFIG_COLOR_OPTION_SUBTITLE_OUTLINE_COLOR "freetype-outline-color"
#define CONFIG_INT_OPTION_SUBTITLE_MARGIN "sub-margin"
#define CONFIG_INT_OPTION_SUBTITLE_OPACITY "freetype-opacity"
#define CONFIG_INT_OPTION_SUBTITLE_OUTLINE_OPACITY "freetype-outline-opacity"
#define CONFIG_INT_OPTION_SUBTITLE_SHADOW_OPACITY "freetype-shadow-opacity"
#define CONFIG_COLOR_OPTION_SUBTITLE_SHADOW_COLOR "freetype-shadow-color"
#define CONFIG_INT_OPTION_SUBTITLE_BACKGROUND_OPACITY "freetype-background-opacity"
#define CONFIG_COLOR_OPTION_SUBTITLE_BACKGROUND_COLOR "freetype-background-color"
#define AOUT_FILTER_EQUALIZER "equalizer"
#define CONFIG_STRING_OPTION_AUDIO_EQUALIZER_PRESET "equalizer-preset"
#define CONFIG_BOOL_OPTION_AUDIO_EQUALIZER_2PASS "equalizer-2pass"
#define CONFIG_STRING_OPTION_AUDIO_DEVICE "audio-device"
#define CONFIG_INT_OPTION_AUDIO_CHANNELS "audio-channels"
#define CONFIG_STRING_OPTION_AUDIO_VISUAL "audio-visual"
#define CONFIG_STRING_OPTION_AUDIO_OUT "audio"


struct libvlc_instance_t;
struct libvlc_media_player_t;
struct libvlc_media_t;
struct libvlc_event_manager_t;
struct libvlc_media_list_t;
struct libvlc_media_list_player_t;
struct libvlc_media_discoverer_t;

#ifdef _MSC_VER
// Typedefs for old MS compilers
typedef unsigned __int32	uint32_t;
typedef __int64             int64_t;
#endif

class DisplayCallback
{
public:
	virtual ~DisplayCallback(){}
	virtual void *vlcLock(void **p_pixels) = 0;

	virtual void vlcUnlock(void *id, void *const *p_pixels) = 0;

	virtual void vlcDisplay(void *id) = 0;
	
};
class InputCallBack
{
public:
	virtual ~InputCallBack(){}
	virtual void vlcPopupCallback(bool show) = 0;
	virtual void vlcFullScreenControlCallback() = 0;
};
class MouseInputCallBack
{
public:
	virtual ~MouseInputCallBack(){}
	virtual void vlcMouseMove(int x, int y, int button) = 0;
	virtual void vlcMouseClick(int x, int y, int button) = 0;
};
class EventCallBack
{
public:
	virtual ~EventCallBack(){}
	virtual void vlcTimeChanged(int64_t newTime) = 0;
	virtual void vlcPaused() = 0;
	virtual void vlcStarted() = 0;
	virtual void vlcStopped() = 0;
};
class AudioCallback
{
public:
	virtual void vlcAudioPlay(const void *samples, unsigned count, int64_t pts)=0;
	virtual void vlcAudioPause(int64_t pts)=0;
	virtual void vlcAudioResume(int64_t pts)=0;
	virtual void vlcAudioFush(int64_t pts)=0;
	virtual void vlcAudioDrain()=0;
};
class VLCUPNPMediaList
{
	
	libvlc_media_discoverer_t*   pMediaDiscoverer_;        ///< The VLC media Discoverer object.
	libvlc_media_list_t* pUPNPMediaList_;
public:
	VLCUPNPMediaList(void);
	~VLCUPNPMediaList(void);

	std::vector<std::pair<std::string, std::string> > getUPNPList(std::vector<std::string> const& path);
};
class VLCWrapper
{
	libvlc_media_player_t*   pMediaPlayer_;        ///< The VLC media player object.
    libvlc_event_manager_t*  pEventManager_;       ///< The event manger for the loaded media file.    
    libvlc_media_list_t *ml;
    libvlc_media_list_player_t *mlp;
	bool m_videoAdjustEnabled;
public:
	VLCWrapper(void);
	~VLCWrapper(void);


    /** Set window for media output.
    *   @param [in] pHwnd window, on Windows a HWND handle. */
    void SetOutputWindow(void* pHwnd);
    void SetDisplayCallback(DisplayCallback* cb);
    void SetEventCallBack(EventCallBack* cb);
    void SetBufferFormat(int imageWidth, int imageHeight, int imageStride);
    void SetInputCallBack(InputCallBack* cb);
    bool setMouseInputCallBack(MouseInputCallBack* cb);
	void SetAudioCallback(AudioCallback* cb);

	void loadSubtitle(const char* pSubPathName);

    /** Start playback. */
    void play();
	
	bool isPaused();
	bool isStopping();
	bool isPlaying();
	bool isStopped();

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

	bool isSeekable();

    /** Mutes the audio output of playback.
    *   @param [in] mute True or false. */
    void Mute(bool mute = true);

    /** Get mute state of playback.
    *   @return True or false. */
    bool GetMute();

    double getVolume();
    void setVolume(double volume);    

	void setScale (double ratio);
	double getScale () const;
	void setRate (double rate);
	double getRate () const;
	void setAspect(const char* ratio);
	std::string getAspect()const;
	void setAudioDelay(int64_t delay);
	int64_t getAudioDelay();
	void setSubtitleDelay(int64_t delay);
	int64_t getSubtitleDelay();
    int getSubtitlesCount();
    int getCurrentSubtitleIndex();
    void setSubtitleIndex(int i);

	
	void setVideoContrast(double n);
	double getVideoContrast();
	void setVideoBrightness(double n);
	double getVideoBrightness();
	void setVideoHue(double n);
	double getVideoHue();
	void setVideoSaturation(double n);
	double getVideoSaturation();
	void setVideoGamma(double n);
	double getVideoGamma();
	void setVideoAdjust(bool n);
	bool getVideoAdjust();
	
	std::vector<std::string> getCropList();
	void setCrop(std::string const& ratio);
	std::string getCrop();
	
	std::vector< std::pair<int, std::string> > getVideoTrackList();
	void setVideoTrack(int n);
	int getVideoTrack();

	std::vector< std::pair<int, std::string> > getAudioTrackList();
	void setAudioTrack(int n);
	int getAudioTrack();

	void setAutoCrop(bool autoCrop);
	bool isAutoCrop();
	
	void setVoutOptionInt(const char* name, int v);
	int getVoutOptionInt(const char* name);

	void setAoutFilterOptionString(const char* name, std::string const&  filter, std::string const& v);
	std::string getAoutFilterOptionString(const char* name);

	enum AudioChannel
	{     
		 VLCWrapperAudioChannel_Error 	
		,VLCWrapperAudioChannel_Stereo 	
		,VLCWrapperAudioChannel_RStereo 	
		,VLCWrapperAudioChannel_Left 	
		,VLCWrapperAudioChannel_Right 	
		,VLCWrapperAudioChannel_Dolbys 	
	};
    AudioChannel getAudioChannel();
    void setAudioChannel(AudioChannel i);

	std::vector<std::string> getCurrentPlayList();
	int addPlayListItem(std::string const& path);
	void playPlayListItem(int index);
	std::string getCurrentPlayListItem();
	int getCurrentPlayListItemIndex();
	void removePlaylistItem(int index);
	void clearPlayList();

	std::string getInfo() const;
	
	bool getConfigOptionBool(const char* name) const;
	void setConfigOptionBool(const char* name, bool value);
	
	int getConfigOptionInt(const char* name) const;
	void setConfigOptionInt(const char* name, int value);
	std::pair<int,std::vector<std::pair<int, std::string> > > getConfigOptionInfoInt(const char* name)const;

	std::string getConfigOptionString(const char* name) const;
	void setConfigOptionString(const char* name, std::string const& value);
	std::pair<std::string,std::vector<std::pair<std::string, std::string> > > getConfigOptionInfoString(const char* name)const;

};

#endif // __VLCWRAPPER_H__