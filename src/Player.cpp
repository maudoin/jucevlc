// Player/Alex Skoruppa (alex.skoruppa@googlemail.com)
// http://www.codeproject.com/info/cpol10.aspx

#include "Player.h"
#include "execute.h"
#include <stdio.h>

Player::Player(juce::DirectShowComponent& dshowComp)
:m_dshowComp(dshowComp)
{

}
std::string Player::getInfo() const
{
	return "JucePlayer";
}
Player::~Player(void)
{
}

void Player::play()
{
    m_dshowComp.play();
}

void Player::Pause()
{
    m_dshowComp.stop();
}

bool Player::isPaused()
{
    return isStopped();
}
bool Player::isPlaying()
{
    return m_dshowComp.isPlaying();
}

bool Player::isStopping()
{
    return false;
}
bool Player::isStopped()
{
    return !m_dshowComp.isMovieOpen();
}

void Player::Stop()
{
    m_dshowComp.closeMovie();
}

int64_t Player::GetLength()
{
    int64_t length = m_dshowComp.getMovieDuration();
    return length;
}

int64_t Player::GetTime()
{
    int64_t time = m_dshowComp.getPosition();
    return time;
}

void Player::SetTime( int64_t newTime )
{
    m_dshowComp.setPosition (newTime);
}

void Player::Mute( bool mute /*= true*/ )
{
    setVolume(0.);
}

bool Player::GetMute()
{
    return getVolume()==0.;
}

double Player::getVolume()
{
    return m_dshowComp.getMovieVolume ();
}

void Player::setVolume( double volume )
{
    m_dshowComp.setMovieVolume (volume);
}
Player::AudioChannel Player::getAudioChannel()
{
    return VLCWrapperAudioChannel_Error;
}
void Player::setAudioChannel(Player::AudioChannel i)
{

}


void Player::loadSubtitle(const char* pSubPathName)
{
}

void Player::setScale (double ratio)
{
}
double Player::getScale() const
{
	return 1.;
}
void Player::setRate (double rate)
{
}
double Player::getRate () const
{
	return 1.;
}
void Player::setAspect(const char* ratio)
{
}
std::string Player::getAspect() const
{
	return std::string("16:9");
}
void Player::setAudioDelay(int64_t delay)
{
}
int64_t Player::getAudioDelay()
{
	return 0;
}
void Player::setSubtitleDelay(int64_t delay)
{
}
int64_t Player::getSubtitleDelay()
{
	return 0;
}
int Player::getSubtitlesCount()
{
	return 0;
}
std::vector<std::pair<int, std::string> > Player::getSubtitles()
{
    std::vector<std::pair<int, std::string> > res;
    return res;
}
int Player::getCurrentSubtitleIndex()
{
	return 0;
}
void Player::setSubtitleIndex(int i)
{
}


std::vector<std::string> Player::getCropList()
{
	std::vector<std::string> list;
	return list;

}
void Player::setCrop(std::string const& ratio)
{
}

std::string Player::getCrop()
{
	std::string crop;
	return crop;
}
void Player::setAutoCrop(bool autoCrop)
{
}

bool Player::isAutoCrop()
{
	bool autoCrop = false;
	return autoCrop;
}

void Player::setVoutOptionInt(const char* name, int v)
{
}

int Player::getVoutOptionInt(const char* name)
{
	int v = 0;
	return v;
}

void Player::setAoutFilterOptionString(const char* name, std::string const& filter, std::string const& v)
{
}

std::string Player::getAoutFilterOptionString(const char* name)
{
}
std::vector< std::pair<int, std::string> > Player::getVideoTrackList()
{
	std::vector< std::pair<int, std::string> > list;
	return list;
}
void Player::setVideoTrack(int n)
{
}
int Player::getVideoTrack()
{
	return 0;
}

void Player::setVideoAdjust(bool n)
{
}
bool Player::getVideoAdjust()
{
	return false;
}
void Player::setVideoContrast(double n)
{
}
double Player::getVideoContrast()
{
	return (double)0.;
}
void Player::setVideoBrightness(double n)
{
}
double Player::getVideoBrightness()
{
	return (double)0.;
}

void Player::setVideoHue(double n)
{
}
double Player::getVideoHue()
{
	return (double)0.;
}
void Player::setVideoSaturation(double n)
{
}
double Player::getVideoSaturation()
{
	return (double)0.;
}
void Player::setVideoGamma(double n)
{
}
double Player::getVideoGamma()
{
	return (double)0.;
}
std::vector< std::pair<int, std::string> > Player::getAudioTrackList()
{
	std::vector< std::pair<int, std::string> > list;
	return list;
}
std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > > Player::getAudioOutputList() const
{
	std::vector< std::pair< std::pair<std::string, std::string>, std::vector< std::pair<std::string, std::string> > > > result;
	return result;
}
void Player::setAudioOutputDevice(std::string const& output, std::string const& device)
{
}

void Player::setAudioTrack(int n)
{
}

int Player::getAudioTrack()
{
	return 0;
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
bool Player::openAndPlay(std::string const& path)
{
    juce::String err;
	bool ok = m_dshowComp.loadMovie (juce::String::fromUTF8(path.c_str()), err);
	if(ok)
    {
        std::string::size_type i = path.find_last_of("/\\");
        m_currentVideoFileName =  i == std::string::npos ? path : path.substr(i+1);
    }
    return ok;
}

std::string Player::getCurrentVideoFileName()const
{
    return m_currentVideoFileName;
}
bool Player::isSeekable()
{
	return true;
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
