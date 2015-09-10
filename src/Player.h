#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <memory>
#include <string>
#include <vector>
//#include <cstdint>

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
class Player
{
	bool m_videoAdjustEnabled;


    EventCallBack* m_pEventCallBack;
    InputCallBack* m_pInputCallBack;
    MouseInputCallBack* m_pMouseInputCallBack;
	AudioCallback* m_pAudioCallback;
public:
	Player(void);
	~Player(void);


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
    std::vector<std::pair<int, std::string> > getSubtitles();
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

	std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > > getAudioOutputList() const;
	void setAudioOutputDevice(std::string const& output, std::string const& device);
};

#endif // __VLCWRAPPER_H__
