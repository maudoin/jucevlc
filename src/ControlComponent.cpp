
#include "ControlComponent.h"
#include "Icons.h"
#include <algorithm>


using namespace std::placeholders;

////////////////////////////////////////////////////////////
//
// 2ND SLIDER
//
////////////////////////////////////////////////////////////


SecondaryControlComponent::SecondaryControlComponent()
	:m_buttonsStep(0.)
	,m_sliderAction([](double){})
{
	setOpaque(true);
	m_leftImage = juce::Drawable::createFromImageData (Icons::left_svg, Icons::left_svgSize);
	m_rightImage = juce::Drawable::createFromImageData (Icons::right_svg, Icons::right_svgSize);

	m_leftButton = std::make_unique<juce::DrawableButton>("m_leftButton", juce::DrawableButton::ImageFitted);
	m_leftButton->setOpaque(false);
	m_leftButton->setImages(m_leftImage.get());
	m_leftButton->addListener(this);

	m_rightButton = std::make_unique<juce::DrawableButton>("m_rightButton", juce::DrawableButton::ImageFitted);
	m_rightButton->setOpaque(false);
	m_rightButton->setImages(m_rightImage.get());
	m_rightButton->addListener(this);

	m_slider = std::make_unique<juce::Slider>("AlternateControlComponentSlider");
	m_slider->addListener(this);
    m_slider->setSliderStyle (Slider::LinearHorizontal);
	m_slider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
	m_slider->setColour(Slider::textBoxTextColourId, juce::Colours::white);
	m_slider->setOpaque(false);
	addAndMakeVisible(*m_slider);

	addChildComponent(*m_leftButton);
	addChildComponent(*m_rightButton);

	setOpaque(false);
}
SecondaryControlComponent::~SecondaryControlComponent()
{
}
void SecondaryControlComponent::resized()
{
	bool showButtons = m_buttonsStep>0.;
	int buttonSize = showButtons?getHeight():0;
	int leftButtonSize= m_leftButton->isVisible() ? buttonSize : 0;
	int rightButtonSize= m_rightButton->isVisible() ? buttonSize : 0;

	int w = getWidth()-leftButtonSize-rightButtonSize;
	int sliderW = (2*w)/3;
	int labelW = w-sliderW;

	if(m_leftButton->isVisible())
	{
		m_leftButton->setBounds(0, 0, buttonSize, getHeight());
	}
	m_slider->setBounds(leftButtonSize, 0, sliderW, getHeight());
	if(m_rightButton->isVisible())
	{
		m_rightButton->setBounds(leftButtonSize+sliderW, 0, buttonSize, getHeight());
	}
	//Rectangle<int> labelBounds(leftButtonSize+sliderW+rightButtonSize, 0, labelW, getHeight());
}

void SecondaryControlComponent::paint(juce::Graphics& g)
{
	//paint label
	bool showButtons = m_buttonsStep>0.;
	int buttonSize = showButtons?getHeight():0;
	int leftButtonSize= m_leftButton->isVisible() ? buttonSize : 0;
	int rightButtonSize= m_rightButton->isVisible() ? buttonSize : 0;

	int w = getWidth()-leftButtonSize-rightButtonSize;
	int sliderW = (2*w)/3;
	int labelW = w-sliderW;

	Rectangle<int> labelBounds(leftButtonSize+sliderW+rightButtonSize, 0, labelW, getHeight());

	juce::Font f = g.getCurrentFont().withHeight(getFontHeight()*3.f/4.f);
	//f.setTypefaceName("Times New Roman");//"Forgotten Futurist Shadow");
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);

	g.setColour (juce::Colours::white);
	g.drawFittedText (juce::String::formatted(m_labelFormat, m_slider->getValue()),
		labelBounds,
		juce::Justification::centredLeft,
		1, //1 line
		1.f//no h scale
		);
}

void SecondaryControlComponent::sliderValueChanged (juce::Slider* slider)
{
	m_sliderAction(slider->getValue());
}

void SecondaryControlComponent::show(juce::String const& label, ActionSliderCallback const& f, double value, double resetValue, double volumeMin, double volumeMax, double step, double buttonsStep)
{
	m_sliderAction = f;
	m_labelFormat = label;
	m_buttonsStep = buttonsStep;
	m_resetValue = resetValue;
	bool showButtons = m_buttonsStep>0.;
	m_leftButton->setVisible(showButtons);
	m_rightButton->setVisible(showButtons);
	m_slider->setRange(volumeMin, volumeMax, step);
	m_slider->setValue(value);
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
	m_sliderAction = [](double){};
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
{
	m_slider = std::make_unique<TimeSlider>();


    m_playImage       = juce::Drawable::createFromImageData (Icons::play_svg,           Icons::play_svgSize);
    m_pauseImage      = juce::Drawable::createFromImageData (Icons::pause_svg,          Icons::pause_svgSize);
    m_stopImage       = juce::Drawable::createFromImageData (Icons::stop_svg,           Icons::stop_svgSize);
    m_itemImage       = juce::Drawable::createFromImageData (Icons::blank_svg,          Icons::blank_svgSize);
    m_folderImage     = juce::Drawable::createFromImageData (Icons::openshort_svg,      Icons::openshort_svgSize);
    m_starImage       = juce::Drawable::createFromImageData (Icons::sliders_svg,        Icons::sliders_svgSize);
    m_fullscreenImage = juce::Drawable::createFromImageData (Icons::fullscreen_svg,     Icons::fullscreen_svgSize);
    m_windowImage     = juce::Drawable::createFromImageData (Icons::window_svg,    		Icons::window_svgSize);
    m_undoImage       = juce::Drawable::createFromImageData (Icons::backCircle_svg,     Icons::backCircle_svgSize);

    m_playPauseButton = std::make_unique<juce::DrawableButton>("playPause", juce::DrawableButton::ImageFitted);
	m_playPauseButton->setOpaque(false);
	m_playPauseButton->setImages(m_playImage.get());

    m_stopButton = std::make_unique<juce::DrawableButton>("stop", juce::DrawableButton::ImageFitted);
	m_stopButton->setOpaque(false);
	m_stopButton->setImages(m_stopImage.get());

    m_fullscreenButton = std::make_unique<juce::DrawableButton>("fullscreenButton", juce::DrawableButton::ImageFitted);
	m_fullscreenButton->setOpaque(false);
	m_fullscreenButton->setImages(m_fullscreenImage.get());
	m_fullscreenButton->setTooltip(TRANS("Switch fullscreen"));

    m_menuButton = std::make_unique<juce::DrawableButton>("menuButton", juce::DrawableButton::ImageFitted);
	m_menuButton->setOpaque(false);
	m_menuButton->setImages(m_folderImage.get());
	m_menuButton->setTooltip(TRANS("Quick menu"));

    m_auxilliarySliderModeButton = std::make_unique<juce::DrawableButton>("2ndSliderModeButton", juce::DrawableButton::ImageFitted);
	m_auxilliarySliderModeButton->setOpaque(false);
	m_auxilliarySliderModeButton->setImages(m_starImage.get());

    m_resetButton = std::make_unique<juce::DrawableButton>("ResetButton", juce::DrawableButton::ImageFitted);
	m_resetButton->setOpaque(false);
	m_resetButton->setImages(m_undoImage.get());
	m_resetButton->setTooltip(TRANS("Reset"));


	m_auxilliaryControlComponent = std::make_unique<SecondaryControlComponent>();

	addAndMakeVisible(*m_slider);
    addAndMakeVisible(*m_playPauseButton);
    addAndMakeVisible(*m_stopButton);
    addAndMakeVisible(*m_fullscreenButton);
    addAndMakeVisible(*m_menuButton);
    addAndMakeVisible(*m_auxilliarySliderModeButton);
    addAndMakeVisible(*m_resetButton);
    addChildComponent(*m_auxilliaryControlComponent);


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
void ControlComponent::setScaleComponent(juce::Component* scaleComponent)
{
	AppProportionnalComponent::setScaleComponent(scaleComponent);
	m_auxilliaryControlComponent->setScaleComponent(scaleComponent);
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
	int auxilliaryW = w - 2*auxilliaryX;
	m_auxilliarySliderModeButton->setBounds (auxilliaryX-buttonSize, h-buttonSize, buttonSize, buttonSize);
	int auxilliaryH = sliderHeight*4/5;
	m_auxilliaryControlComponent->setBounds (auxilliaryX, h-buttonSize+(buttonSize-auxilliaryH)/2, auxilliaryW, auxilliaryH);

	m_resetButton->setBounds (auxilliaryX+m_auxilliaryControlComponent->getWidth(), h-buttonSize, buttonSize, buttonSize);

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

	return h == 0 ? juce::String::formatted("%02d:%02d", m, s) : juce::String::formatted("%02d:%02d:%02d", h, m, s);
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