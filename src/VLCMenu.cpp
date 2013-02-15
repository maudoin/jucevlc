#include "VideoComponent.h"
#include "VLCMenu.h"
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>


class Action : public AbstractAction
{
    typedef boost::function<void (MenuTreeItem&)> Functor;
    Functor m_f;
public:
    explicit Action (Functor const& f) : m_f (f) { }
    void operator() (MenuTreeItem& parent) { m_f (parent); }
	
	////////////////////////////////////// FUNCTION
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&))
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1));
	}
	template <typename P1>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1), P1 p1)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1));
	}
	template <typename P1, typename P2>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1, P2), P1 p1, P2 p2)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2));
	}
	template <typename P1, typename P2, typename P3>
	static AbstractAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, P1, P2, P3), P1 p1, P2 p2, P3 p3)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3));
	}
	////////////////////////////////////// MEMBER
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&))
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1));
	}
	template <typename P1>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1), P1 p1)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1));
	}
	template <typename P1, typename P2>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2), P1 p1, P2 p2)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2));
	}
	template <typename P1, typename P2, typename P3>
	static AbstractAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, P1, P2, P3), P1 p1, P2 p2, P3 p3)
	{
		return new Action(boost::bind<void>(f, boost::ref(video), _1, p1, p2, p3));
	}
};

class FileAction : public AbstractFileAction
{
    typedef boost::function<void (MenuTreeItem&, juce::File const&)> Functor;
    Functor m_f;
public:
    explicit FileAction (Functor const& f) : m_f (f) { }
    explicit FileAction (FileAction const& a) : m_f (a.m_f) { }
	AbstractFileAction* clone() const{return new FileAction(*this);}
    void operator() (MenuTreeItem& parent, juce::File const&file) { m_f (parent, file); }
	
	////////////////////////////////////// FUNCTION
	static AbstractFileAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, juce::File const&))
	{
		return new FileAction(boost::bind<void>(f, boost::ref(video), _1, _2));
	}
	template <typename P1>
	static AbstractFileAction* build (VideoComponent &video, void (*f)(VideoComponent&, MenuTreeItem&, juce::File const&, P1), P1 p1)
	{
		return new FileAction(boost::bind<void>(f, boost::ref(video), _1, _2, p1));
	}

	////////////////////////////////////// MEMBER
	static AbstractFileAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, juce::File const&))
	{
		return new FileAction(boost::bind<void>(f, boost::ref(video), _1, _2));
	}
	template <typename P1>
	static AbstractFileAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, juce::File const&, P1), P1 p1)
	{
		return new FileAction(boost::bind<void>(f, boost::ref(video), _1, _2, p1));
	}
	template <typename P1, typename P2>
	static AbstractFileAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, juce::File const&, P1, P2), P1 p1, P2 p2)
	{
		return new FileAction(boost::bind<void>(f, boost::ref(video), _1, _2, p1, p2));
	}
	template <typename P1, typename P2, typename P3>
	static AbstractFileAction* build (VideoComponent &video, void (VideoComponent::*f)(MenuTreeItem&, juce::File const&, P1, P2, P3), P1 p1, P2 p2, P3 p3)
	{
		return new FileAction(boost::bind<void>(f, boost::ref(video), _1, _2, p1, p2, p3));
	}
};
void nop(VideoComponent &video, MenuTreeItem& parent)
{
}
void listFiles(VideoComponent &video, MenuTreeItem& item, AbstractFileAction* fileMethod)
{
	juce::Array<juce::File> destArray;
	juce::File::findFileSystemRoots(destArray);
	item.addFiles(destArray, fileMethod);
}


void pin(VideoComponent &video, MenuTreeItem& item)
{
	item.addAction ("Pin", Action::build(video, &nop));
}

void volume(VideoComponent &video, MenuTreeItem& item)
{
	item.addAction( "free", Action::build(video, &pin));
	item.addAction( "25%", Action::build(video, &VideoComponent::onAudioVolume, 25));
	item.addAction( "50%", Action::build(video, &VideoComponent::onAudioVolume, 50));
	item.addAction( "75%", Action::build(video, &VideoComponent::onAudioVolume, 75));
	item.addAction( "100%", Action::build(video, &VideoComponent::onAudioVolume, 100));
	item.addAction( "125%", Action::build(video, &VideoComponent::onAudioVolume, 125));
	item.addAction( "150%", Action::build(video, &VideoComponent::onAudioVolume, 150));
	item.addAction( "175%", Action::build(video, &VideoComponent::onAudioVolume, 175));
	item.addAction( "200%", Action::build(video, &VideoComponent::onAudioVolume, 200));
}
void soundOptions(VideoComponent &video, MenuTreeItem& item)
{
	item.addAction( "Volume", Action::build(video, &volume));
	item.addAction( "Shift", Action::build(video, &pin));
	item.addAction( "Mute", Action::build(video, &pin));
}

void crop(VideoComponent &video, MenuTreeItem& item)
{
	item.addAction( "free", Action::build(video, &pin));
	item.addAction( "16/10", Action::build(video, &VideoComponent::onCrop, 16.f/10.f));
	item.addAction( "16/9", Action::build(video, &VideoComponent::onCrop, 16.f/9.f));
	item.addAction( "4/3", Action::build(video, &VideoComponent::onCrop, 4.f/3.f));
}
void ratio(VideoComponent &video, MenuTreeItem& item)
{
	item.addAction( "original", Action::build(video, &VideoComponent::onSetAspectRatio, juce::String("")));
	item.addAction( "16/10", Action::build(video, &VideoComponent::onSetAspectRatio, juce::String("16/10")));
	item.addAction( "16/9", Action::build(video, &VideoComponent::onSetAspectRatio, juce::String("16/9")));
	item.addAction( "4/3", Action::build(video, &VideoComponent::onSetAspectRatio, juce::String("4/3")));
	
}
void rate(VideoComponent &video, MenuTreeItem& item)
{
	item.addAction( "free", Action::build(video, &pin));
	item.addAction( "50%", Action::build(video, &VideoComponent::onRate, .5f));
	item.addAction( "100%", Action::build(video, &VideoComponent::onRate, 1.f));
	item.addAction( "150%", Action::build(video, &VideoComponent::onRate, 1.5f));
	item.addAction( "200%", Action::build(video, &VideoComponent::onRate, 2.f));
	item.addAction( "300%", Action::build(video, &VideoComponent::onRate, 3.f));
	item.addAction( "400%", Action::build(video, &VideoComponent::onRate, 4.f));
	item.addAction( "600%", Action::build(video, &VideoComponent::onRate, 6.f));
	item.addAction( "800%", Action::build(video, &VideoComponent::onRate, 8.f));
}
void videoOptions(VideoComponent &video, MenuTreeItem& item)
{
	item.addAction( "FullScreen", Action::build(video, &VideoComponent::onFullscreen, true));
	item.addAction( "Windowed", Action::build(video, &VideoComponent::onFullscreen, false));
	item.addAction( "Speed", Action::build(video, &rate));
	item.addAction( "Crop", Action::build(video, &crop));
	item.addAction( "Ratio", Action::build(video, &ratio));
}
void subtitlesOptions(VideoComponent &video, MenuTreeItem& item)
{
	item.addAction( "Select", Action::build(video, &nop));
	item.addAction( "Add...", Action::build(video, &listFiles, FileAction::build(video, &VideoComponent::onOpenSubtitle)));
}
void exitVLCFrontend(VideoComponent &video, MenuTreeItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
void getRootITems(VideoComponent &video, MenuTreeItem& item)
{
	item.addAction( "Open", Action::build(video, &listFiles, FileAction::build(video, &VideoComponent::onOpen)), video.getFolderShortcutImage());
	item.addAction( "Subtitle", Action::build(video, &subtitlesOptions), video.getSubtitlesImage());
	item.addAction( "Video options", Action::build(video, &videoOptions), video.getDisplayImage());
	item.addAction( "Sound options", Action::build(video, &soundOptions), video.getAudioImage());
	item.addAction( "Exit", Action::build(video, &exitVLCFrontend), video.getExitImage());

}

AbstractAction* getVideoRootMenu(VideoComponent &video)
{
	return Action::build(video, &getRootITems);
}