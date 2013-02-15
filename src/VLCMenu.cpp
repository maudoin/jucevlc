#include "MenuTree.h"
#include "VLCMenu.h"
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>

template <typename P1>
void dispatchToListeners (MenuTree &tree, MenuTreeItem& item, void (MenuTreeListener::*callbackFunction) (P1), P1 p1)
{
	tree.getListeners().call(callbackFunction, p1);
}
template <typename P1, typename P2>
void dispatchToListeners (MenuTree &tree, MenuTreeItem& item, void (MenuTreeListener::*callbackFunction) (P1, P2), P1 p1, P2 p2)
{
	tree.getListeners().call(callbackFunction, p1, p2);
}

void nop(MenuTree &tree, MenuTreeItem& parent)
{
}
void listFiles(MenuTree &tree, MenuTreeItem& item, MenuTreeListener::FileMethod fileMethod)
{
	juce::Array<juce::File> destArray;
	juce::File::findFileSystemRoots(destArray);
	item.addFiles(destArray, fileMethod);
}


void pin(MenuTree &tree, MenuTreeItem& item)
{
	item.addAction ("Pin", Action::build(tree, &nop));
}

void volume(MenuTree &tree, MenuTreeItem& item)
{
	item.addAction( "free", Action::build(tree, &pin));
	item.addAction( "25%", Action::build(tree, &dispatchToListeners<int>, &MenuTreeListener::onAudioVolume, 25));
	item.addAction( "50%", Action::build(tree, &dispatchToListeners<int>, &MenuTreeListener::onAudioVolume, 50));
	item.addAction( "75%", Action::build(tree, &dispatchToListeners<int>, &MenuTreeListener::onAudioVolume, 75));
	item.addAction( "100%", Action::build(tree, &dispatchToListeners<int>, &MenuTreeListener::onAudioVolume, 100));
	item.addAction( "125%", Action::build(tree, &dispatchToListeners<int>, &MenuTreeListener::onAudioVolume, 125));
	item.addAction( "150%", Action::build(tree, &dispatchToListeners<int>, &MenuTreeListener::onAudioVolume, 150));
	item.addAction( "175%", Action::build(tree, &dispatchToListeners<int>, &MenuTreeListener::onAudioVolume, 175));
	item.addAction( "200%", Action::build(tree, &dispatchToListeners<int>, &MenuTreeListener::onAudioVolume, 200));
}
void soundOptions(MenuTree &tree, MenuTreeItem& item)
{
	item.addAction( "Volume", Action::build(tree, &volume));
	item.addAction( "Shift", Action::build(tree, &pin));
	item.addAction( "Mute", Action::build(tree, &pin));
}

void crop(MenuTree &tree, MenuTreeItem& item)
{
	item.addAction( "free", Action::build(tree, &pin));
	item.addAction( "16/10", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onCrop, 16.f/10.f));
	item.addAction( "16/9", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onCrop, 16.f/9.f));
	item.addAction( "4/3", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onCrop, 4.f/3.f));
}
void ratio(MenuTree &tree, MenuTreeItem& item)
{
	item.addAction( "original", Action::build(tree, &dispatchToListeners<juce::String>, &MenuTreeListener::onSetAspectRatio, juce::String("")));
	item.addAction( "16/10", Action::build(tree, &dispatchToListeners<juce::String>, &MenuTreeListener::onSetAspectRatio, juce::String("16/10")));
	item.addAction( "16/9", Action::build(tree, &dispatchToListeners<juce::String>, &MenuTreeListener::onSetAspectRatio, juce::String("16/9")));
	item.addAction( "4/3", Action::build(tree, &dispatchToListeners<juce::String>, &MenuTreeListener::onSetAspectRatio, juce::String("4/3")));
	
}
void rate(MenuTree &tree, MenuTreeItem& item)
{
	item.addAction( "free", Action::build(tree, &pin));
	item.addAction( "50%", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onRate, .5f));
	item.addAction( "100%", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onRate, 1.f));
	item.addAction( "150%", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onRate, 1.5f));
	item.addAction( "200%", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onRate, 2.f));
	item.addAction( "300%", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onRate, 3.f));
	item.addAction( "400%", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onRate, 4.f));
	item.addAction( "600%", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onRate, 6.f));
	item.addAction( "800%", Action::build(tree, &dispatchToListeners<float>, &MenuTreeListener::onRate, 8.f));
}
void videoOptions(MenuTree &tree, MenuTreeItem& item)
{
	item.addAction( "FullScreen", Action::build(tree, &dispatchToListeners<bool>, &MenuTreeListener::onFullscreen, true));
	item.addAction( "Windowed", Action::build(tree, &dispatchToListeners<bool>, &MenuTreeListener::onFullscreen, false));
	item.addAction( "Speed", Action::build(tree, &rate));
	item.addAction( "Crop", Action::build(tree, &crop));
	item.addAction( "Ratio", Action::build(tree, &ratio));
}
void subtitlesOptions(MenuTree &tree, MenuTreeItem& item)
{
	item.addAction( "Select", Action::build(tree, &nop));
	item.addAction( "Add", Action::build(tree, &listFiles, &MenuTreeListener::onOpenSubtitle));
}
void exitVLCFrontend(MenuTree &tree, MenuTreeItem& item)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
void getRootITems(MenuTree &tree, MenuTreeItem& item)
{
	item.addAction( "Open", Action::build(tree, &listFiles, &MenuTreeListener::onOpen), tree.getFolderShortcutImage());
	item.addAction( "Subtitle", Action::build(tree, &subtitlesOptions), tree.getSubtitlesImage());
	item.addAction( "Video options", Action::build(tree, &videoOptions), tree.getDisplayImage());
	item.addAction( "Sound options", Action::build(tree, &soundOptions), tree.getAudioImage());
	item.addAction( "Exit", Action::build(tree, &exitVLCFrontend), tree.getExitImage());

}