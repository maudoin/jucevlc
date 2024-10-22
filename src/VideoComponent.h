
#ifndef VIDEO_COMPONENT
#define VIDEO_COMPONENT

#include "AbstractMenu.h"
#include "AppProportionnalComponent.h"
#include "ControlComponent.h"
#include "LookNFeel.h"
#include "PlayerMenus.h"
#include "VLCWrapper.h"

#include <JuceHeader.h>
#include <sstream>
#include <set>


#define BUFFER_DISPLAY
#undef BUFFER_DISPLAY

class InvokeLater;
namespace
{
class TitleComponent;
class BackgoundUPNP;
}
class VideoComponent   : public juce::Component , public juce::KeyListener,

#ifdef BUFFER_DISPLAY
	DisplayCallback,
#else
	juce::ComponentListener,
#endif
	juce::Slider::Listener, juce::Button::Listener, EventCallBack, InputCallBack, MouseInputCallBack, juce::TimeSliceClient,
	public PlayerMenus::ViewHandler
{
	bool m_canHideOSD;
	juce::TimeSliceThread m_backgroundTasks;
	std::unique_ptr<AbstractMenu> m_fileMenu;
    std::unique_ptr<AbstractMenu> m_optionsMenu;
    std::unique_ptr<PlayerMenus> m_videoPlayerEngine;

#ifdef BUFFER_DISPLAY
	std::unique_ptr<juce::Image> img;
	std::unique_ptr<juce::Image::BitmapData> ptr;
#else
    std::unique_ptr<juce::Component> vlcNativePopupComponent;
#endif
    std::unique_ptr<juce::Component> m_toolTip;
    std::unique_ptr<ControlComponent> controlComponent;
    juce::CriticalSection imgCriticalSection;
	std::unique_ptr<VLCWrapper> vlc;
	bool sliderUpdating;
	bool videoUpdating;
    std::unique_ptr<juce::Drawable> settingsImage;
    std::unique_ptr<juce::Drawable> openSettingsImage;
    std::unique_ptr<juce::Drawable> audioImage;
    std::unique_ptr<juce::Drawable> appImage;
	LnF lnf;
    std::unique_ptr<TitleComponent> titleBar;
    std::unique_ptr<juce::ResizableBorderComponent> resizableBorder;
    juce::ComponentBoundsConstrainer defaultConstrainer;
	juce::int64 lastMouseMoveMovieTime;
    std::unique_ptr<InvokeLater> invokeLater;

public:
    VideoComponent(const juce::String& commandLine);
    virtual ~VideoComponent();


    int useTimeSlice() override;

	void handleFullRelayout() override;
	void handleControlRelayout() override;
	void closeFileMenu() override;
	void playPlayListItem(int index, std::string const& name) override;
	bool isFullScreen() const final;
	void setFullScreen(bool fs) final;

	void play();
	void pause();
	void stop();
	void rewindTime ();
	void advanceTime ();
	void switchFullScreen();
	void switchPlayPause();
	void setupVolumeSlider(double value);

#ifdef BUFFER_DISPLAY
	//VLC DiaplListener
	void *vlcLock(void **p_pixels);
	void vlcUnlock(void *id, void *const *p_pixels);
	void vlcDisplay(void *id);
#else
    void componentMovedOrResized(Component& component,bool wasMoved, bool wasResized) override;
    void componentVisibilityChanged(Component& component) override;
#endif

	/////////////// VLC EvtListener
	void vlcTimeChanged(int64_t newTime) override;
	void vlcPaused() override;
	void vlcStarted() override;
	void vlcStopped() override;
	void vlcPopupCallback(bool show) override;
	void vlcFullScreenControlCallback() override;
	void vlcMouseMove(int x, int y, int button) override;
	void vlcMouseClick(int x, int y, int button) override;

	void startedSynchronous();
	void stoppedSynchronous();


	/////////////// GUI CALLBACKS
    void paint (juce::Graphics& g) override;

    void resized() override;
	void updateTimeAndSlider(int64_t newTime);

    void sliderValueChanged (juce::Slider* slider) override;
    void buttonClicked (juce::Button* button) override;
	void userTriedToCloseWindow() override;
	using juce::Component::keyPressed;
	bool keyPressed (const juce::KeyPress& key,
								juce::Component* originatingComponent) override;
    void mouseDown (const juce::MouseEvent& e) override;
	void mouseDrag (const juce::MouseEvent& e) override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseWheelMove (const juce::MouseEvent& e,
                                 const juce::MouseWheelDetails& wheel) override;

	//void minimisationStateChanged (bool isNowMinimised){if(!isNowMinimised)resized();}
    void broughtToFront() override;

	enum SliderModeButton
	{
		  E_POPUP_ITEM_VOLUME_SLIDER = 1
		, E_POPUP_ITEM_SUBTITLES_DELAY_SLIDER
		, E_POPUP_ITEM_VOLUME_DELAY_SLIDER
		, E_POPUP_ITEM_PLAY_SPEED_SLIDER
		, E_POPUP_ITEM_ZOOM_SLIDER
	};
private:

	void handleIdleTimeAndControlsVisibility();
	void setMenuTreeVisibleAndUpdateMenuButtonIcon(bool visible);
	void updateOptionMenuBounds();
	void updateSubComponentsBounds();

	void forceSetVideoTime(int64_t start);
	void forceSetVideoTime(std::string const& name);
	bool isFrontpageVisible();

    bool downloadedSubtitleSeekerResult(MenuComponentValue const&, juce::String const& resultSite,
                                                    char* cstr,
                                                     juce::String const& siteTarget,
                                                     std::string const& match,
                                                     std::string const& downloadURLPattern );

};

#endif //VIDEO_COMPONENT
