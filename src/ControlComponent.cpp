
#include "ControlComponent.h"
#include "Icons.h"
#include "MenuTree.h"
#include "MenuTreeAction.h"
#include <algorithm>

////////////////////////////////////////////////////////////
//
// 2ND SLIDER
//
////////////////////////////////////////////////////////////
#include <boost/bind/bind.hpp>

void nop(double)
{
}
class ActionSlider   : public SliderWithInnerLabel, public juce::SliderListener
{
public:
    typedef boost::function<void (double)> Functor;
private:
	Functor m_f;
public:
	ActionSlider(juce::String const& name="")
		:SliderWithInnerLabel(name)
		,m_f(boost::bind(&nop, _1))
	{
		addListener(this);
	}
	virtual ~ActionSlider(){}
	void resetCallback(){m_f = boost::bind(&nop, _1);}
	void setCallback(ActionSliderCallback const& f){m_f = f;}
    virtual void sliderValueChanged (juce::Slider* slider)
	{
		m_f(slider->getValue());
	}
	void disable()
	{
		setCallback(boost::bind(&nop, _1));
	}
	void setup(juce::String const& label, ActionSliderCallback const& f, double value, double volumeMin, double volumeMax, double step)
	{
		setRange(volumeMin, volumeMax, step);
		setValue(value);
		setLabelFormat(label);
		setCallback(f);
	}
};

SecondaryControlComponent::SecondaryControlComponent()
	:m_buttonsStep(0.)
{
	setOpaque(true);
	m_leftImage = juce::Drawable::createFromImageData (left_svg, left_svgSize);
	m_rightImage = juce::Drawable::createFromImageData (right_svg, right_svgSize);

	m_leftButton = new juce::DrawableButton("m_leftButton", juce::DrawableButton::ImageFitted);
	m_leftButton->setOpaque(false);
	m_leftButton->setImages(m_leftImage);
	m_leftButton->addListener(this);

	m_rightButton = new juce::DrawableButton("m_rightButton", juce::DrawableButton::ImageFitted);
	m_rightButton->setOpaque(false);
	m_rightButton->setImages(m_rightImage);
	m_rightButton->addListener(this);

	m_slider = new ActionSlider("AlternateControlComponentSlider");
	addAndMakeVisible(m_slider);

	addChildComponent(m_leftButton);
	addChildComponent(m_rightButton);

	setOpaque(false);
}
SecondaryControlComponent::~SecondaryControlComponent()
{
}
void SecondaryControlComponent::resized()
{
	bool showButtons = m_buttonsStep>0.;
	int buttonSize = showButtons?getHeight():0;

	int leftButtonSize=0;
	if(m_leftButton->isVisible())
	{
		leftButtonSize=buttonSize;
		m_leftButton->setBounds(0, 0, buttonSize, getHeight());
	}
	int rightButtonSize=0;
	if(m_rightButton->isVisible())
	{
		rightButtonSize=buttonSize;
		m_rightButton->setBounds(getWidth()-buttonSize, 0, buttonSize, getHeight());
	}
	m_slider->setBounds(leftButtonSize, 0, getWidth()-leftButtonSize-rightButtonSize, getHeight());
}
	
void SecondaryControlComponent::paint(juce::Graphics& g)
{
}
void SecondaryControlComponent::show(juce::String const& label, ActionSliderCallback const& f, double value, double resetValue, double volumeMin, double volumeMax, double step, double buttonsStep)
{
	m_buttonsStep = buttonsStep;
	m_resetValue = resetValue;
	bool showButtons = m_buttonsStep>0.;
	m_leftButton->setVisible(showButtons);
	m_rightButton->setVisible(showButtons);
	m_slider->setup(label, f, value, volumeMin, volumeMax, step);
	resized();
	setVisible(true);
}
	
void SecondaryControlComponent::reset()
{
	double min = m_slider->getMinimum();
	double max = m_slider->getMaximum();
	if(m_resetValue < min || m_resetValue > max)
	{
		double fix = std::ceil((m_resetValue - (min + max) / 2.  )/m_buttonsStep)*m_buttonsStep;
		m_slider->setRange(min+fix, max+fix, m_slider->getInterval());
	}
	m_slider->setValue(m_resetValue);
}
void SecondaryControlComponent::disableAndHide()
{
	m_slider->disable();
	setVisible(false);
}
void SecondaryControlComponent::buttonClicked (juce::Button* button)
{
	double value = m_slider->getValue();
	double min = m_slider->getMinimum();
	double max = m_slider->getMaximum();
	double step = m_slider->getInterval();

	if( button == m_leftButton.get() )
	{
		m_slider->setRange(min-m_buttonsStep, max-m_buttonsStep, step);
		m_slider->setValue(std::min(value, max-m_buttonsStep));
	}
	else if( button == m_rightButton.get() )
	{
		m_slider->setRange(min+m_buttonsStep, max+m_buttonsStep, step);
		m_slider->setValue(std::max(value, min-m_buttonsStep));
	}
}
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
	setOpaque(true);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
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
	mouseOverTimeString = juce::String::empty;
	mouseOverTimeStringPos = -1;
	repaint();
}
	
//juce GUI overrides
void TimeSlider::paint (juce::Graphics& g)
{
	juce::Slider::paint(g);
	
	if(mouseOverTimeStringPos>0)
	{
		juce::Font f = g.getCurrentFont().withHeight(getFontHeight());
		f.setTypefaceName("Times New Roman");//"Forgotten Futurist Shadow");
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);

		g.setColour (juce::Colours::black);

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
							xText, 0,widthText,getHeight(),
							justification, 
							1, //1 line
							1.f//no h scale
							);

		g.drawLine(mouseOverTimeStringPos, 0, mouseOverTimeStringPos, getHeight(), 2);
	}

	
}
////////////////////////////////////////////////////////////
//
// CONTROL COMPONENT
//
////////////////////////////////////////////////////////////
ControlComponent::ControlComponent()
{
	m_slider = new TimeSlider();
	
	
    m_playImage = juce::Drawable::createFromImageData (play_svg, play_svgSize);
    m_pauseImage = juce::Drawable::createFromImageData (pause_svg, pause_svgSize);
    m_stopImage = juce::Drawable::createFromImageData (stop_svg, stop_svgSize);
    m_itemImage = juce::Drawable::createFromImageData (blue_svg, blue_svgSize);
    m_folderImage = juce::Drawable::createFromImageData (folderShortcut_svg, folderShortcut_svgSize);
    m_starImage = juce::Drawable::createFromImageData (star_svg, star_svgSize);
    m_fullscreenImage = juce::Drawable::createFromImageData (fullscreen_svg, fullscreen_svgSize);
    m_undoImage = juce::Drawable::createFromImageData (undo_svg, undo_svgSize);

    m_playPauseButton = new juce::DrawableButton("playPause", juce::DrawableButton::ImageFitted);
	m_playPauseButton->setOpaque(false);
	m_playPauseButton->setImages(m_playImage);

    m_stopButton = new juce::DrawableButton("stop", juce::DrawableButton::ImageFitted);
	m_stopButton->setOpaque(false);
	m_stopButton->setImages(m_stopImage);	

    m_fullscreenButton = new juce::DrawableButton("fullscreenButton", juce::DrawableButton::ImageFitted);
	m_fullscreenButton->setOpaque(false);
	m_fullscreenButton->setImages(m_fullscreenImage);
	m_fullscreenButton->setTooltip(TRANS("Switch fullscreen"));

    m_menuButton = new juce::DrawableButton("menuButton", juce::DrawableButton::ImageFitted);
	m_menuButton->setOpaque(false);
	m_menuButton->setImages(m_folderImage);
	m_menuButton->setTooltip(TRANS("Quick menu"));

    m_auxilliarySliderModeButton = new juce::DrawableButton("2ndSliderModeButton", juce::DrawableButton::ImageFitted);
	m_auxilliarySliderModeButton->setOpaque(false);
	m_auxilliarySliderModeButton->setImages(m_starImage);

    m_resetButton = new juce::DrawableButton("ResetButton", juce::DrawableButton::ImageFitted);
	m_resetButton->setOpaque(false);
	m_resetButton->setImages(m_undoImage);
	m_resetButton->setTooltip(TRANS("Reset"));
	

	m_auxilliaryControlComponent = new SecondaryControlComponent();
	
	addAndMakeVisible(m_slider);
    addAndMakeVisible(m_playPauseButton);
    addAndMakeVisible(m_stopButton);
    addAndMakeVisible(m_fullscreenButton);
    addAndMakeVisible(m_menuButton);
    addAndMakeVisible(m_auxilliarySliderModeButton);
    addAndMakeVisible(m_resetButton);
    addChildComponent(m_auxilliaryControlComponent);

	
	setOpaque(false);
}
ControlComponent::~ControlComponent()
{
	m_slider = nullptr;
	m_playPauseButton = nullptr;
	m_stopButton = nullptr;
	m_auxilliarySliderModeButton = nullptr;
	m_resetButton = nullptr;
	m_playImage = nullptr;
	m_pauseImage = nullptr;
	m_stopImage = nullptr;
}
void ControlComponent::resized()
{
	int w =  getWidth();
	int h =  getHeight();

	
	int buttonSize = 0.5*h;
	int hMargin =buttonSize/2;
	int sliderHeight = 0.3*h;
	int playPauseButtonSize = buttonSize+sliderHeight;
	int sliderLeftMargin = hMargin + playPauseButtonSize;
	m_slider->setBounds (sliderLeftMargin, h-sliderHeight-buttonSize, w-sliderLeftMargin-hMargin, sliderHeight);

	m_playPauseButton->setBounds (hMargin, h-playPauseButtonSize, playPauseButtonSize, playPauseButtonSize);
	m_stopButton->setBounds (hMargin+playPauseButtonSize, h-buttonSize, buttonSize, buttonSize);	
	m_fullscreenButton->setBounds (hMargin+playPauseButtonSize+buttonSize, h-buttonSize, buttonSize, buttonSize);
	m_menuButton->setBounds (hMargin+playPauseButtonSize+2*buttonSize, h-buttonSize, buttonSize, buttonSize);

	int auxilliaryX = playPauseButtonSize+6*buttonSize;
	int auxilliaryW = w - 2*auxilliaryX;
	m_auxilliarySliderModeButton->setBounds (auxilliaryX-buttonSize, h-buttonSize, buttonSize, buttonSize);
	m_auxilliaryControlComponent->setBounds (auxilliaryX, h-buttonSize+(buttonSize-sliderHeight)/2, auxilliaryW, sliderHeight);

	m_resetButton->setBounds (auxilliaryX+m_auxilliaryControlComponent->getWidth(), h-buttonSize, buttonSize, buttonSize);
}
void ControlComponent::paint(juce::Graphics& g)
{
	int w =  getWidth();
	int h =  getHeight();

	
	int buttonSize = 0.5*h;
	int hMargin =buttonSize/2;
	int sliderHeight = 0.3*h;
	int roundness = hMargin/2;
	
	///////////////// CONTROL ZONE:	
	g.setGradientFill (juce::ColourGradient (juce::Colours::darkgrey.withAlpha(0.5f),
										w/2, h-sliderHeight-buttonSize-hMargin/2,
										juce::Colours::black,
										w/2, h,
										false));
	g.fillRoundedRectangle(hMargin/2,  h-sliderHeight-buttonSize-hMargin/2, w-hMargin, sliderHeight+buttonSize+hMargin/2, roundness);

	g.setGradientFill (juce::ColourGradient (juce::Colours::lightgrey.withAlpha(0.5f),
										w/2, h-sliderHeight-buttonSize-hMargin/2,
										juce::Colours::black,
										w/2, h-hMargin/2,
										false));
	g.drawRoundedRectangle(hMargin/2,  h-sliderHeight-buttonSize-hMargin/2, w-hMargin, sliderHeight+buttonSize+hMargin/2, roundness,2.f);
	
	///////////////// TIME:
	juce::Font f = g.getCurrentFont().withHeight(getFontHeight());
	f.setTypefaceName("Times New Roman");//"Forgotten Futurist Shadow");
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);

	g.setColour (juce::Colours::white);

	

	g.drawFittedText (timeString,
						hMargin+2*buttonSize, h-buttonSize, w-2*hMargin-2*buttonSize, buttonSize,
						juce::Justification::topRight, 
						1, //1 line
						1.f//no h scale
						);

	if(! (m_auxilliaryControlComponent && m_auxilliaryControlComponent->isVisible()))
	{
		g.drawFittedText (currentTimeString, 
			m_auxilliaryControlComponent->getBounds(),
			juce::Justification::centred, 
			1, //1 line
			1.f//no h scale
			);
	}
	
}

juce::String toString(juce::int64 time)
{
	int h = (int)(time/(1000*60*60) );
	int m = (int)(time/(1000*60) - 60*h );
	int s = (int)(time/(1000) - 60*m - 60*60*h );
	
	return juce::String::formatted("%02d:%02d:%02d", h, m, s);
}

void ControlComponent::setTime(juce::int64 time, juce::int64 len)
{
	timeString = toString(time) + "/" + toString(len);

	juce::int64 current = juce::Time::currentTimeMillis();
	juce::int64 eta = current + len - time;

	currentTimeString = juce::Time(current).formatted("%H:%M") + juce::Time(eta).formatted(" -> %H:%M");
}

void ControlComponent::showPausedControls()
{
	m_playPauseButton->setImages(m_playImage);

	setVisible(true);
}
void ControlComponent::showPlayingControls()
{
	m_playPauseButton->setImages(m_pauseImage);
	
	setVisible(true);
}
void ControlComponent::hidePlayingControls()
{
	setVisible(false);
}