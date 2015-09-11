// Player/Alex Skoruppa (alex.skoruppa@googlemail.com)
// http://www.codeproject.com/info/cpol10.aspx

#include "Player.h"
#include "execute.h"
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <windows.h>
#include <ShellApi.h>
#include <winuser.h>


enum MPC_LOADSTATE {
	MLS_CLOSED,
	MLS_LOADING,
	MLS_LOADED,
	MLS_CLOSING
};
enum MPC_PLAYSTATE {
	PS_PLAY   = 0,
	PS_PAUSE  = 1,
	PS_STOP   = 2,
	PS_UNUSED = 3
};
struct MPC_OSDDATA {
	int nMsgPos;       // screen position constant (see OSD_MESSAGEPOS constants)
	int nDurationMS;   // duration in milliseconds
	TCHAR strMsg[128]; // message to display thought OSD
};
//// MPC_OSDDATA.nMsgPos constants (for host side programming):
//typedef enum
//{
//    OSD_NOMESSAGE,
//    OSD_TOPLEFT,
//    OSD_TOPRIGHT,
//} OSD_MESSAGEPOS;
typedef enum MPCAPI_COMMAND {
	// ==== Commands from MPC to host

	// Send after connection
	// Par 1 : MPC window handle (command should be send to this HWnd)
	CMD_CONNECT				= 0x50000000,
	// Send when opening or closing file
	// Par 1 : current state (see MPC_LOADSTATE enum)
	CMD_STATE				= 0x50000001,
	// Send when playing, pausing or closing file
	// Par 1 : current play mode (see MPC_PLAYSTATE enum)
	CMD_PLAYMODE			= 0x50000002,
	// Send after opening a new file
	// Par 1 : title
	// Par 2 : author
	// Par 3 : description
	// Par 4 : complete filename (path included)
	// Par 5 : duration in seconds
	CMD_NOWPLAYING			= 0x50000003,
	// List of subtitle tracks
	// Par 1 : Subtitle track name 0
	// Par 2 : Subtitle track name 1
	// ...
	// Par n : Active subtitle track, -1 if subtitles disabled
	//
	// if no subtitle track present, returns -1
	// if no file loaded, returns -2
	CMD_LISTSUBTITLETRACKS		= 0x50000004,
	// List of audio tracks
	// Par 1 : Audio track name 0
	// Par 2 : Audio track name 1
	// ...
	// Par n : Active audio track
	//
	// if no audio track present, returns -1
	// if no file loaded, returns -2
	CMD_LISTAUDIOTRACKS			= 0x50000005,
	// Send current playback position in responce
	// of CMD_GETCURRENTPOSITION.
	// Par 1 : current position in seconds
	CMD_CURRENTPOSITION			= 0x50000007,
	// Send the current playback position after a jump.
	// (Automatically sent after a seek event).
	// Par 1 : new playback position (in seconds).
	CMD_NOTIFYSEEK				= 0x50000008,
	// Notify the end of current playback
	// (Automatically sent).
	// Par 1 : none.
	CMD_NOTIFYENDOFSTREAM		= 0x50000009,
	// List of files in the playlist
	// Par 1 : file path 0
	// Par 2 : file path 1
	// ...
	// Par n : active file, -1 if no active file
	CMD_PLAYLIST				= 0x50000006,

	// ==== Commands from host to MPC
	// Open new file
	// Par 1 : file path
	CMD_OPENFILE			= 0xA0000000,
	// Stop playback, but keep file / playlist
	CMD_STOP				= 0xA0000001,
	// Stop playback and close file / playlist
	CMD_CLOSEFILE			= 0xA0000002,
	// Pause or restart playback
	CMD_PLAYPAUSE			= 0xA0000003,
	// Add a new file to playlist (did not start playing)
	// Par 1 : file path
	CMD_ADDTOPLAYLIST		= 0xA0001000,
	// Remove all files from playlist
	CMD_CLEARPLAYLIST		= 0xA0001001,
	// Start playing playlist
	CMD_STARTPLAYLIST		= 0xA0001002,
	CMD_REMOVEFROMPLAYLIST	= 0xA0001003,	// TODO
	// Cue current file to specific position
	// Par 1 : new position in seconds
	CMD_SETPOSITION			= 0xA0002000,
	// Set the audio delay
	// Par 1 : new audio delay in ms
	CMD_SETAUDIODELAY		= 0xA0002001,
	// Set the subtitle delay
	// Par 1 : new subtitle delay in ms
	CMD_SETSUBTITLEDELAY	= 0xA0002002,
	// Set the active file in the playlist
	// Par 1 : index of the active file, -1 for no file selected
	// DOESN'T WORK
	CMD_SETINDEXPLAYLIST	= 0xA0002003,
	// Set the audio track
	// Par 1 : index of the audio track
	CMD_SETAUDIOTRACK		= 0xA0002004,
	// Set the subtitle track
	// Par 1 : index of the subtitle track, -1 for disabling subtitles
	CMD_SETSUBTITLETRACK	= 0xA0002005,
	// Ask for a list of the subtitles tracks of the file
	// return a CMD_LISTSUBTITLETRACKS
	CMD_GETSUBTITLETRACKS		= 0xA0003000,
	// Ask for the current playback position,
	// see CMD_CURRENTPOSITION.
	// Par 1 : current position in seconds
	CMD_GETCURRENTPOSITION		= 0xA0003004,
	// Jump forward/backward of N seconds,
	// Par 1 : seconds (negative values for backward)
	CMD_JUMPOFNSECONDS			= 0xA0003005,
	// Ask for a list of the audio tracks of the file
	// return a CMD_LISTAUDIOTRACKS
	CMD_GETAUDIOTRACKS			= 0xA0003001,
	// Ask for the properties of the current loaded file
	// return a CMD_NOWPLAYING
	CMD_GETNOWPLAYING			= 0xA0003002,
	// Ask for the current playlist
	// return a CMD_PLAYLIST
	CMD_GETPLAYLIST				= 0xA0003003,
	// Toggle FullScreen
	CMD_TOGGLEFULLSCREEN		= 0xA0004000,
	// Jump forward(medium)
	CMD_JUMPFORWARDMED			= 0xA0004001,
	// Jump backward(medium)
	CMD_JUMPBACKWARDMED			= 0xA0004002,
	// Increase Volume
	CMD_INCREASEVOLUME			= 0xA0004003,
	// Decrease volume
	CMD_DECREASEVOLUME			= 0xA0004004,
	// Shader toggle
	CMD_SHADER_TOGGLE			= 0xA0004005,
	// Close App
	CMD_CLOSEAPP				= 0xA0004006,
	// show host defined OSD message string
	CMD_OSDSHOWMESSAGE			= 0xA0005000,
};

PlayerImpl* g_hideousGlobal = 0;
LRESULT CALLBACK windowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
struct PlayerImpl
{
	HWND m_targetHwnd;
    WNDPROC m_originalWndProc;
    HWND m_hWndMPC;

	PlayerImpl()
    : m_targetHwnd(0)
    , m_originalWndProc(0)
    , m_hWndMPC(0)
	{
	}
	~PlayerImpl()
	{

        if(m_targetHwnd && m_originalWndProc)
        {
            SetWindowLongPtr (m_targetHwnd, GWLP_WNDPROC, (LONG_PTR) m_originalWndProc);
            m_originalWndProc = 0;
        }
	}
	void hook(HWND pHwnd)
	{

       m_targetHwnd = (HWND)pHwnd;
       m_originalWndProc = ((WNDPROC) GetWindowLongPtr (m_targetHwnd, GWLP_WNDPROC));

       g_hideousGlobal = this;
       SetWindowLongPtr(m_targetHwnd, GWLP_WNDPROC, (LONG_PTR) ::windowProc);

	}
	void sendCommand(MPCAPI_COMMAND command, unsigned long size, void* pointer)
	{
        COPYDATASTRUCT MyCDS;
        MyCDS.dwData = command;
        MyCDS.cbData = size;  // size of data
        MyCDS.lpData = pointer;           // data structure

        SendMessage( (HWND)m_hWndMPC,
                       WM_COPYDATA,
                       (WPARAM)(HWND) m_targetHwnd,
                       (LPARAM) (LPVOID) &MyCDS );
	}
    LRESULT windowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};

LRESULT CALLBACK windowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(g_hideousGlobal)
    {
        return g_hideousGlobal->windowProc(hwnd, message, wParam, lParam);
    }
    else
    {
        return DefWindowProcW (hwnd, message, wParam, lParam);
    }
}
LRESULT PlayerImpl::windowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    if(message == WM_COPYDATA)
    {
        COPYDATASTRUCT* pCopyDataStruct = (COPYDATASTRUCT*) lParam;

        if (pCopyDataStruct->dwData == CMD_CONNECT)
        {
            m_hWndMPC = (HWND)(void*)boost::lexical_cast<int>((LPWSTR)pCopyDataStruct->lpData);

            MPC_OSDDATA osd;
            osd.nMsgPos = 1;       // screen position constant (see OSD_MESSAGEPOS constants)
            osd.nDurationMS = 1500000;   // duration in milliseconds
            strcpy(osd.strMsg, "---- Juced! ----"); // message to display thought OSD

            sendCommand(CMD_OSDSHOWMESSAGE,//CMD_CLOSEAPP,//
                        sizeof(MPC_OSDDATA),
                        (void*)&osd );

        }
    }

    return CallWindowProc (m_originalWndProc, hwnd, message, wParam, lParam);
} // msghook


Player::Player()
:m_private(new PlayerImpl())
{

}
std::string Player::getInfo() const
{
	return "JucePlayer";
}
Player::~Player(void)
{
    delete m_private;
}

void Player::SetDisplayCallback(DisplayCallback* cb)
{
}


void Player::SetAudioCallback(AudioCallback* cb)
{
}

void Player::SetEventCallBack(EventCallBack* cb)
{
}
void Player::SetBufferFormat(int imageWidth, int imageHeight, int imageStride)
{
}
void Player::SetOutputWindow(void* pHwnd)
{
   m_private->hook((HWND)pHwnd);

    std::string params = (std::string("/slave ")+boost::lexical_cast<std::string>((unsigned int)m_private->m_targetHwnd)).c_str();
    execute("C:\\Program Files (x86)\\MPC-HC\\mpc-hc.exe", "C:\\Program Files (x86)\\MPC-HC", params.c_str());

}

void Player::play()
{
}

void Player::Pause()
{
}

bool Player::isPaused()
{
    return false;
}
bool Player::isPlaying()
{
    return true;
}

bool Player::isStopping()
{
    return false;
}
bool Player::isStopped()
{
    return false;
}

void Player::Stop()
{
}

int64_t Player::GetLength()
{
    int64_t length = 500;
    return length;
}

int64_t Player::GetTime()
{
    int64_t time = 0;
    return time;
}

void Player::SetTime( int64_t newTime )
{
}

void Player::Mute( bool mute /*= true*/ )
{
}

bool Player::GetMute()
{
    return false;
}

double Player::getVolume()
{
    return 0.;
}

void Player::setVolume( double volume )
{
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

void Player::SetInputCallBack(InputCallBack* cb)
{
}





bool Player::setMouseInputCallBack(MouseInputCallBack* cb)
{
	return false;
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
std::vector<std::string> Player::getCurrentPlayList()
{
	std::vector<std::string> out;
	return out;
}
void Player::removePlaylistItem(int index)
{
}
void Player::clearPlayList()
{
}
int Player::addPlayListItem(std::string const& path)
{
    std::wstring widestr = std::wstring(path.begin(), path.end());
    m_private->sendCommand(CMD_ADDTOPLAYLIST, sizeof(wchar_t)*widestr.size(), (void*)widestr.c_str() );
	return 1;
}
std::string Player::getCurrentPlayListItem()
{
	return "???";
}

bool Player::isSeekable()
{
	return false;
}
int Player::getCurrentPlayListItemIndex()
{
	return 0;
}

void Player::playPlayListItem(int index)
{
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
