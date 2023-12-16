#ifndef LOOKANDFEEL
#define LOOKANDFEEL

#include <JuceHeader.h>
#include "FontSerialization.h"
#include "AppProportionnalComponent.h"
#include "WeblySleekFont.h"


class LnF : public juce::LookAndFeel_V1
{
	juce::Typeface* cFont;

public:
	LnF()
	{
		setColour(juce::DirectoryContentsDisplayComponent::textColourId, juce::Colours::white);
#ifdef _DEBUG
		//serializeFont("Forgotten Futurist Shadow", "ForgottenFuturistShadow.bin.new");
		//serializeFont("Teen", "FontResource.cpp", "FontResource.h");
		serializeFont("WeblySleek UI Semilight", "src/WeblySleekFont.cpp", "src/WeblySleekFont.h");
#endif
		//cFont = loadFont( "font.bin");
		cFont = loadFont( WeblySleek_UI_SemilightData, WeblySleek_UI_SemilightSize);
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