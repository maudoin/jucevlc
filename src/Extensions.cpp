
#include "Extensions.h"


Extensions::Extensions()
{
	EXTENSIONS_VIDEO([this](const char* item){m_videoExtensions.insert(item);});
	EXTENSIONS_PLAYLIST([this](const char* item){m_playlistExtensions.insert(item);});
	EXTENSIONS_SUBTITLE([this](const char* item){m_subtitlesExtensions.insert(item);});

	m_supportedExtensions.push_back(m_videoExtensions);
	m_supportedExtensions.push_back(m_subtitlesExtensions);
	m_supportedExtensions.push_back(m_playlistExtensions);
}

Extensions& Extensions::get()
{
	static Extensions instance;
	return instance;
}