#ifndef LOOKANDFEEL
#define LOOKANDFEEL

#include "juce.h"
#include "FontSerialization.h"
#include "AppProportionnalComponent.h"
#include "FontResource.h"


class LnF : public juce::OldSchoolLookAndFeel
{
	juce::Typeface* cFont;

public:
	LnF()
	{
		setColour(juce::DirectoryContentsDisplayComponent::textColourId, juce::Colours::white);
#ifdef _DEBUG
		//serializeFont("Forgotten Futurist Shadow", "ForgottenFuturistShadow.bin.new");
		serializeFont("Teen", "FontResource.cpp", "FontResource.h");
#endif
		//cFont = loadFont( "font.bin");
		cFont = loadFont( TeenData, TeenSize);
	}
	const juce::Typeface::Ptr getTypefaceForFont (const juce::Font &font)
	{
		if (cFont)
		{
			return (cFont);
		}
		else
		{
			return (font.getTypeface());
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
		g.setColour (juce::Colours::grey);
		g.fillRect(x, y, width, height);
		LookAndFeel::drawLinearSlider (g,
                                   x, y,
                                   width, height,
                                   sliderPos,
                                   minSliderPos,
                                   maxSliderPos,
                                   style,
                                   slider);
	}
};


#endif