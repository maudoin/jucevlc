#ifndef LOOKANDFEEL
#define LOOKANDFEEL

#include <JuceHeader.h>
#include "AppProportionnalComponent.h"
#include "Fonts.h"


class LnF : public juce::LookAndFeel_V1
{
	juce::Typeface::Ptr cFont;

public:
	LnF()
	{
		setColour(juce::DirectoryContentsDisplayComponent::textColourId, juce::Colours::white);
		cFont = juce::Typeface::createSystemTypefaceFor(Fonts::NotoSansRegular_ttf, Fonts::NotoSansRegular_ttfSize);
	}
	juce::Typeface::Ptr getTypefaceForFont (const juce::Font &font)
	{
		if (cFont)
		{
			return (cFont);
		}
		else
		{
			return (font.getTypefacePtr());
		}
	}

    virtual void drawLinearSlider (juce::Graphics& g,
                                   int x, int y,
                                   int width, int height,
                                   float sliderPos,
                                   float minSliderPos,
                                   float maxSliderPos,
                                   const juce::Slider::SliderStyle style,
                                   juce::Slider& slider)
	{
		g.fillAll (slider.findColour (Slider::backgroundColourId));

		if (style == Slider::LinearBar)
		{
			int h = height/4;
			int barY = y+height-h;

			g.setColour (slider.findColour (Slider::trackColourId));
			g.fillRect(x, barY,
					   width, h);

			g.setColour (slider.findColour (Slider::thumbColourId));
			g.fillRect (x, barY, (int) sliderPos - x, h);
		}
		else
		{
			LookAndFeel_V1::drawLinearSlider (g,
									x, y,
									width, height,
									sliderPos,
									minSliderPos,
									maxSliderPos,
									style,
									slider);
		}
	}
};


#endif