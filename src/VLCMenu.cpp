#include "VideoComponent.h"
#include "MenuTreeAction.h"
void nop(VideoComponent &video, MenuTreeItem& parent)
{
}

void soundOptions(VideoComponent &video, MenuTreeItem& item)
{
	item.focusItemAsMenuShortcut();
	item.addAction( "Volume", Action::build(video, &VideoComponent::onAudioVolumeSlider, 1., 200.));
	item.addAction( "Shift", Action::build(video, &nop));
}

void ratio(VideoComponent &video, MenuTreeItem& item)
{
	item.focusItemAsMenuShortcut();
	item.addAction( "original", Action::build(video, &VideoComponent::onSetAspectRatio, juce::String("")));
	item.addAction( "16/10", Action::build(video, &VideoComponent::onSetAspectRatio, juce::String("16/10")));
	item.addAction( "16/9", Action::build(video, &VideoComponent::onSetAspectRatio, juce::String("16/9")));
	item.addAction( "4/3", Action::build(video, &VideoComponent::onSetAspectRatio, juce::String("4/3")));
	
}
void videoOptions(VideoComponent &video, MenuTreeItem& item)
{
	item.focusItemAsMenuShortcut();
	item.addAction( "FullScreen", Action::build(video, &VideoComponent::onFullscreen, true));
	item.addAction( "Windowed", Action::build(video, &VideoComponent::onFullscreen, false));
	item.addAction( "Speed", Action::build(video, &VideoComponent::onRateSlider, 1., 800.));
	item.addAction( "Zoom", Action::build(video, &VideoComponent::onCropSlider, 50., 200.));
	item.addAction( "Aspect Ratio", Action::build(video, &ratio));
}
void exitVLCFrontend(VideoComponent &video, MenuTreeItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
void getRootITems(VideoComponent &video, MenuTreeItem& item)
{
	item.focusItemAsMenuShortcut();
	item.addAction( "Open", Action::build(video, &VideoComponent::onListFiles, FileAction::build(video, &VideoComponent::onOpen)), video.getFolderShortcutImage());
	item.addAction( "Subtitle", Action::build(video, &VideoComponent::onSubtitleMenu), video.getSubtitlesImage());
	item.addAction( "Video options", Action::build(video, &videoOptions), video.getDisplayImage());
	item.addAction( "Sound options", Action::build(video, &soundOptions), video.getAudioImage());
	item.addAction( "Exit", Action::build(video, &exitVLCFrontend), video.getExitImage());

}

AbstractAction* getVideoRootMenu(VideoComponent &video)
{
	return Action::build(video, &getRootITems);
}