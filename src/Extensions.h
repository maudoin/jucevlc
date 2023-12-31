#pragma once

#include <JuceHeader.h>
#include <set>
#include <vector>

template<typename Op>
void EXTENSIONS_VIDEO(Op const& add_)
{
	add_("3g2");add_("3gp");add_("3gp2");add_("3gpp");add_("amv");add_("asf");add_("avi");add_("bin");add_("divx");add_("drc");add_("dv");add_("f4v");add_("flv");add_("gxf");add_("iso");add_("m1v");add_("m2v");
	add_("m2t");add_("m2ts");add_("m4v");add_("mkv");add_("mov");add_("mp2");add_("mp2v");add_("mp4");add_("mp4v");add_("mpa");add_("mpe");add_("mpeg");add_("mpeg1");
	add_("mpeg2");add_("mpeg4");add_("mpg");add_("mpv2");add_("mts");add_("mtv");add_("mxf");add_("mxg");add_("nsv");add_("nuv");
	add_("ogg");add_("ogm");add_("ogv");add_("ogx");add_("ps");
	add_("rec");add_("rm");add_("rmvb");add_("tod");add_("ts");add_("tts");add_("vob");add_("vro");add_("webm");add_("wm");add_("wmv");add_("flv");
}

template<typename Op>
void EXTENSIONS_PLAYLIST(Op const& add_)
{
	add_("asx");add_("b4s");add_("cue");add_("ifo");add_("m3u");add_("m3u8");add_("pls");add_("ram");add_("rar");add_("sdp");add_("vlc");add_("xspf");add_("wvx");add_("zip");add_("conf");
}

template<typename Op>
void EXTENSIONS_SUBTITLE(Op const& add_)
{
	add_("cdg");add_("idx");add_("srt");
	add_("sub");add_("utf");add_("ass");
	add_("ssa");add_("aqt");
	add_("jss");add_("psb");
	add_("rt");add_("smi");add_("txt");
	add_("smil");add_("stl");add_("usf");
	add_("dks");add_("pjs");add_("mpl2");
	add_("vtt");
}

class Extensions
{
	std::set<juce::String> m_videoExtensions;
	std::set<juce::String> m_playlistExtensions;
	std::set<juce::String> m_subtitlesExtensions;
	std::vector< std::set<juce::String> > m_supportedExtensions;

	Extensions();
public:

	std::set<juce::String> const& videoExtensions() const{return m_videoExtensions;}
	std::set<juce::String> const& playlistExtensions() const{return m_playlistExtensions;}
	std::set<juce::String> const& subtitlesExtensions() const{return m_subtitlesExtensions;}
	std::vector< std::set<juce::String> > const& supportedExtensions() const{return m_supportedExtensions;}
	static Extensions& get();
};