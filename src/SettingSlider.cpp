
#include "SettingSlider.h"
#include "Icons.h"
#include <algorithm>


using namespace std::placeholders;

////////////////////////////////////////////////////////////
//
// 2ND SLIDER
//
////////////////////////////////////////////////////////////


SettingSlider::SettingSlider()
	: m_leftImage(juce::Drawable::createFromImageData (Icons::left_svg, Icons::left_svgSize))
	, m_rightImage(juce::Drawable::createFromImageData (Icons::right_svg, Icons::right_svgSize))
	, m_resetImage(juce::Drawable::createFromImageData (Icons::backCircle_svg, Icons::backCircle_svgSize))
	, m_leftButton(std::make_unique<juce::DrawableButton>("m_leftButton", juce::DrawableButton::ImageFitted))
	, m_rightButton(std::make_unique<juce::DrawableButton>("m_rightButton", juce::DrawableButton::ImageFitted))
	, m_resetButton(std::make_unique<juce::DrawableButton>("m_resetButton", juce::DrawableButton::ImageFitted))
	, m_slider(std::make_unique<juce::Slider>("AlternateControlComponentSlider"))
	, m_buttonsStep(0.)
{
	setOpaque(true);

	m_leftButton->setOpaque(false);
	m_leftButton->setImages(m_leftImage.get());
	m_leftButton->addListener(this);
	addChildComponent(*m_rightButton);

	m_rightButton->setOpaque(false);
	m_rightButton->setImages(m_rightImage.get());
	m_rightButton->addListener(this);
	addChildComponent(*m_leftButton);

	m_resetButton->setOpaque(false);
	m_resetButton->setImages(m_resetImage.get());
	m_resetButton->setTooltip(TRANS("Reset"));
	m_resetButton->addListener(this);
	addAndMakeVisible(*m_resetButton);

    m_slider->setSliderStyle (Slider::LinearHorizontal);
	m_slider->setTextBoxStyle (Slider::NoTextBox, false, 80, 20);
	m_slider->setColour(Slider::textBoxTextColourId, juce::Colours::white);
	m_slider->setOpaque(false);
	addAndMakeVisible(*m_slider);



	setOpaque(false);
}

SettingSlider::~SettingSlider()
{
}

void SettingSlider::addListener(juce::Slider::Listener* l)
{
	m_slider->addListener(l);
}

void SettingSlider::resized()
{
	int buttonSize = getHeight();
	int leftButtonSize= m_leftButton->isVisible() ? buttonSize : 0;
	int rightButtonSize= m_rightButton->isVisible() ? buttonSize : 0;

	int w = getWidth()-leftButtonSize-rightButtonSize-buttonSize;
	int sliderW = (2*w)/3;

	if(m_leftButton->isVisible())
	{
		m_leftButton->setBounds(0, 0, buttonSize, getHeight());
	}
	m_slider->setBounds(leftButtonSize, 0, sliderW, getHeight());
	if(m_rightButton->isVisible())
	{
		m_rightButton->setBounds(leftButtonSize+sliderW, 0, buttonSize, getHeight());
	}
	m_resetButton->setBounds(getWidth()-buttonSize, 0, buttonSize, getHeight());
}

void SettingSlider::paint(juce::Graphics& g)
{
	//paint label
	bool showButtons = m_buttonsStep>0.;
	int buttonSize = showButtons?getHeight():0;
	int leftButtonSize= m_leftButton->isVisible() ? buttonSize : 0;
	int rightButtonSize= m_rightButton->isVisible() ? buttonSize : 0;

	int w = getWidth()-leftButtonSize-rightButtonSize-buttonSize;
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


void SettingSlider::setup(SettingSlider::Params const&params)
{
	m_labelFormat = params.label;
	m_buttonsStep = params.arrowsStep;
	m_resetValue = params.defaultVal;
	m_slider->setRange(params.min, params.max, params.step);
	m_slider->setValue(params.init);
	bool showButtons = m_buttonsStep>0.;
	m_leftButton->setVisible(showButtons);
	m_rightButton->setVisible(showButtons);
	resized();
}

void SettingSlider::buttonClicked (juce::Button* button)
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
	else if( button == m_resetButton.get() )
	{
		if(m_resetValue < min || m_resetValue > max)
		{
			double fix = std::ceil((m_resetValue - (min + max) / 2.  )/m_buttonsStep)*m_buttonsStep;
			m_slider->setRange(min+fix, max+fix, m_slider->getInterval());
		}
		m_slider->setValue(m_resetValue);
	}
}


double SettingSlider::getValue()const
{
	return m_slider->getValue();
}