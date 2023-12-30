
#include "ControlComponent.h"
#include "Icons.h"
#include "SettingSlider.h"
#include <algorithm>


using namespace std::placeholders;

////////////////////////////////////////////////////////////
//
// TimeSlider
//
////////////////////////////////////////////////////////////

TimeSlider::TimeSlider()
	: juce::Slider("Time slider")
	,mouseOverTimeStringPos(-1)
{
	setRange(0, 10000);
	setSliderStyle (juce::Slider::LinearBar);
	setAlpha(1.f);
	setOpaque(false);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
	setColour(Slider::trackColourId, juce::Colours::white);
	setColour(Slider::thumbColourId, juce::Colours::red);
	//setPopupDisplayEnabled(true, this);//useless as it shows seconds and not on mouse over!
	//setChangeNotificationOnlyOnRelease(true);//if we use this vlc callback make the slider "forget" the dragged position...

}
TimeSlider::~TimeSlider()
{
}


void TimeSlider::setMouseOverTime(int pos, juce::int64 time)
{
	mouseOverTimeString = toString(time);
	mouseOverTimeStringPos = pos;
	repaint();
}
void TimeSlider::resetMouseOverTime()
{
	mouseOverTimeString.clear();
	mouseOverTimeStringPos = -1;
	repaint();
}

//juce GUI overrides
void TimeSlider::paint (juce::Graphics& g)
{
	g.fillAll (findColour (Slider::backgroundColourId));

	auto layout = LookAndFeel::getDefaultLookAndFeel().getSliderLayout (*this);
	auto sliderRect = layout.sliderBounds;
	int h = sliderRect.getHeight()/4;
	int barY = sliderRect.getY()+sliderRect.getHeight()-h;
	int sliderPos = getPositionOfValue(getValue());

	g.setColour (findColour (Slider::trackColourId));
	g.fillRect(sliderRect.getX(), barY,
				sliderRect.getWidth(), h);

	g.setColour (findColour (Slider::thumbColourId));
	g.fillRect (sliderRect.getX(), barY, (int) sliderPos - sliderRect.getX(), h);

	if(mouseOverTimeStringPos>0)
	{
		juce::Font f = g.getCurrentFont().withHeight(getFontHeight()*3.f/4.f);
		//f.setTypefaceName("Times New Roman");//"Forgotten Futurist Shadow");
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);

		g.setColour (juce::Colours::white);

		int xText = mouseOverTimeStringPos + 3;//add some margin from caret
		int widthText = getWidth()-xText;
		int textSize = f.getStringWidth(mouseOverTimeString);
		juce::Justification justification = juce::Justification::centredLeft;
		if(xText+textSize>getWidth())
		{
			widthText = mouseOverTimeStringPos - 3;
			xText = 0;
			justification = juce::Justification::centredRight;
		}
		g.drawFittedText (mouseOverTimeString,
							xText, 0,widthText,(getHeight()*3)/4,
							justification,
							1, //1 line
							1.f//no h scale
							);


		if (mouseOverTimeStringPos > sliderPos)
		{

			int h = sliderRect.getHeight()/4;
			int barY = sliderRect.getY()+sliderRect.getHeight()-h;

			g.setColour (juce::Colours::grey);
			g.fillRect (sliderPos, barY, mouseOverTimeStringPos-sliderPos, h);
		}

	}


}
////////////////////////////////////////////////////////////
//
// CONTROL COMPONENT
//
////////////////////////////////////////////////////////////
ControlComponent::ControlComponent()
	: m_slider( std::make_unique<TimeSlider>())
    , m_playPauseButton (std::make_unique<juce::DrawableButton>("playPause", juce::DrawableButton::ImageFitted))
    , m_stopButton (std::make_unique<juce::DrawableButton>("stop", juce::DrawableButton::ImageFitted))
    , m_menuButton (std::make_unique<juce::DrawableButton>("menuButton", juce::DrawableButton::ImageFitted))
    , m_fullscreenButton (std::make_unique<juce::DrawableButton>("fullscreen", juce::DrawableButton::ImageFitted))
    , m_volumeButton (std::make_unique<juce::DrawableButton>("volume", juce::DrawableButton::ImageFitted))
    , m_playImage       (juce::Drawable::createFromImageData (Icons::play_svg,           Icons::play_svgSize))
    , m_pauseImage      (juce::Drawable::createFromImageData (Icons::pause_svg,          Icons::pause_svgSize))
    , m_stopImage       (juce::Drawable::createFromImageData (Icons::stop_svg,           Icons::stop_svgSize))
    , m_settingsImage   (juce::Drawable::createFromImageData (Icons::settings_svg,       Icons::settings_svgSize))
    , m_audioImage      (juce::Drawable::createFromImageData (Icons::audio_svg,          Icons::audio_svgSize))
    , m_fullscreenImage (juce::Drawable::createFromImageData (Icons::fullscreen_svg,     Icons::fullscreen_svgSize))
    , m_windowImage     (juce::Drawable::createFromImageData (Icons::window_svg,         Icons::window_svgSize))
	, m_volumeSlider(std::make_unique<juce::Slider>())
	, timeString("")
	, m_volumeSliderAction([](double){})
{
	m_slider->addListener(this);


	m_playPauseButton->setOpaque(false);
	m_playPauseButton->setImages(m_playImage.get());

	m_stopButton->setOpaque(false);
	m_stopButton->setImages(m_stopImage.get());

	m_fullscreenButton->setOpaque(false);
	m_fullscreenButton->setImages(m_fullscreenImage.get());
	m_fullscreenButton->setTooltip(TRANS("Switch fullscreen"));

	m_menuButton->setOpaque(false);
	m_menuButton->setImages(m_settingsImage.get());
	m_menuButton->setTooltip(TRANS("Quick menu"));

	m_volumeButton->setOpaque(false);
	m_volumeButton->setImages(m_audioImage.get());

    m_volumeSlider->setSliderStyle (Slider::LinearHorizontal);
	m_volumeSlider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
	m_volumeSlider->setVisible(true);
	m_volumeSlider->addListener(this);

	addAndMakeVisible(*m_slider);
    addAndMakeVisible(*m_playPauseButton);
    addAndMakeVisible(*m_stopButton);
    addAndMakeVisible(*m_fullscreenButton);
    addAndMakeVisible(*m_menuButton);
    addAndMakeVisible(*m_volumeButton);
    addChildComponent(*m_volumeSlider);


	setOpaque(false);
}
ControlComponent::~ControlComponent()
{
	m_slider = nullptr;
	m_playPauseButton = nullptr;
	m_stopButton = nullptr;
	m_volumeButton = nullptr;
	m_playImage = nullptr;
	m_pauseImage = nullptr;
	m_stopImage = nullptr;
}

void ControlComponent::setupVolumeSlider(VolumeSliderCallback const& f,  double value, double min, double max, double step)
{
	m_volumeSliderAction = f;
	m_volumeSlider->setRange({min, max}, step);
	m_volumeSlider->setValue(value);
}

void ControlComponent::setScaleComponent(juce::Component* scaleComponent)
{
	AppProportionnalComponent::setScaleComponent(scaleComponent);
	m_slider->setScaleComponent(scaleComponent);
}
void ControlComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();


	int buttonSize = h/2;
 	int wMargin = buttonSize/2;
	int sliderHeight = (int)(0.25*h);
	int sliderLeftMargin = wMargin;
	m_slider->setBounds (sliderLeftMargin, h-sliderHeight-buttonSize, w-sliderLeftMargin-wMargin, sliderHeight);

	m_playPauseButton->setBounds (wMargin, h-buttonSize, buttonSize, buttonSize);
	m_stopButton->setBounds (wMargin+buttonSize, h-buttonSize, buttonSize, buttonSize);

	int auxilliaryX = wMargin+7*buttonSize;
	int auxilliaryW = 4*buttonSize;
	m_volumeButton->setBounds (auxilliaryX-buttonSize, h-buttonSize, buttonSize, buttonSize);
	int auxilliaryH = sliderHeight*4/5;
	m_volumeSlider->setBounds (auxilliaryX, h-buttonSize+(buttonSize-auxilliaryH)/2, auxilliaryW, auxilliaryH);

	m_fullscreenButton->setBounds (w-wMargin-2*buttonSize, h-buttonSize, buttonSize, buttonSize);
	m_menuButton->setBounds (w-wMargin-buttonSize, h-buttonSize, buttonSize, buttonSize);
}
void ControlComponent::paint(juce::Graphics& g)
{
	float w = (float)getWidth();
	float h = (float)getHeight();


	float buttonSize = 0.5f*h;
 	float hMargin = buttonSize/22.f;
 	float wMargin = buttonSize/2.f;
	float sliderHeight = 0.3f*h;

	///////////////// CONTROL ZONE:
	g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha(0.f),
										w/2.f, h-sliderHeight-buttonSize-hMargin/2.f,
										juce::Colours::black,
										w/2.f, h,
										false));
	g.fillRect(0.f,  h-sliderHeight-buttonSize-hMargin/2.f, w, h);


	///////////////// TIME:
	juce::Font f = g.getCurrentFont().withHeight(getFontHeight());
	//f.setTypefaceName("Times New Roman");//"Forgotten Futurist Shadow");
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);

	g.setColour (juce::Colours::white);



	g.drawFittedText (timeString,
						(int)(wMargin+2*buttonSize), (int)(h-buttonSize), (int)(w-2*wMargin-2*buttonSize), (int)(buttonSize),
						juce::Justification::left,
						1, //1 line
						1.f//no h scale
						);

}

juce::String toString(juce::int64 time)
{
	int h = (int)(time/(1000*60*60) );
	int m = (int)(time/(1000*60) - 60*h );
	int s = (int)(time/(1000) - 60*m - 60*60*h );

	return h == 0 ? juce::String::formatted("%02d:%02d", m, s) : juce::String::formatted("%02d:%02d:%02d", h, m, s);
}

void ControlComponent::setTime(juce::String const& timeString)
{
	this->timeString = timeString;
}

void ControlComponent::showPausedControls()
{
	m_playPauseButton->setImages(m_playImage.get());

	setVisible(true);
}
void ControlComponent::showPlayingControls()
{
	m_playPauseButton->setImages(m_pauseImage.get());

	setVisible(true);
}

void ControlComponent::showFullscreenControls()
{
	m_fullscreenButton->setImages(m_windowImage.get());

	setVisible(true);
}
void ControlComponent::showWindowedControls()
{
	m_fullscreenButton->setImages(m_fullscreenImage.get());

	setVisible(true);
}
void ControlComponent::hidePlayingControls()
{
	setVisible(false);
}

void ControlComponent::sliderValueChanged (Slider* slider)
{
	if(m_volumeSlider.get() == slider && m_volumeSlider->isVisible())
	{
		m_volumeSliderAction(slider->getValue());
	}
}