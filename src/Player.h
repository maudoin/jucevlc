#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "AppConfig.h"
#include "juce.h"
#include <memory>
#include <string>
#include <vector>
#include <stdint.h>
#include <boost/logic/tribool.hpp>

class Player
{
    juce::DirectShowComponent& m_dshowComp;
	bool m_videoAdjustEnabled;
	std::string m_currentVideoFileName;


public:
	Player(juce::DirectShowComponent& dshowComp);
	~Player(void);

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

	bool openAndPlay(std::string const& path);
	std::string getCurrentVideoFileName()const;

	std::string getInfo() const;

	std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > > getAudioOutputList() const;
	void setAudioOutputDevice(std::string const& output, std::string const& device);
};

#endif // __VLCWRAPPER_H__
