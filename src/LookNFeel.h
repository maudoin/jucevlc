#ifndef LOOKANDFEEL
#define LOOKANDFEEL

#include "juce.h"
#include "FontSerialization.h"
#include "AppProportionnalComponent.h"


class LnF : public juce::OldSchoolLookAndFeel
{
	juce::Typeface* cFont;

public:
	LnF()
	{
		setColour(juce::DirectoryContentsDisplayComponent::textColourId, juce::Colours::white);
#ifdef _DEBUG
		//serializeFont("Forgotten Futurist Shadow", "ForgottenFuturistShadow.bin.new");
		serializeFont("Teen", "font.bin");
#endif
		cFont = loadFont( "font.bin");
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
		
		LookAndFeel::drawLinearSlider (g,
                                   x, y,
                                   width, height,
                                   sliderPos,
                                   minSliderPos,
                                   maxSliderPos,
                                   style,
                                   slider);
	}
	void drawListRow (juce::Graphics& g, int width, int height,const juce::String& name,
										  bool isItemSelected)
	{
		float fontSize =  0.9f*height;
		const int filenameWidth = width;//width > 450 ? roundToInt (width * 0.7f) : width;
	
		float hborder = height/8.f;
		float roundness = height/2.f;
	
	
	
		if(isItemSelected)
		{
			g.setGradientFill (juce::ColourGradient (isItemSelected?findColour (juce::DirectoryContentsDisplayComponent::highlightColourId):juce::Colours::darkgrey.darker(),
											   0.f, height/2.f-hborder,
											   juce::Colour (0x8000),
											   0.7f*filenameWidth-3.f, height/2.f-hborder,
											   false));

			//g.setColour (isItemSelected?findColour (DirectoryContentsDisplayComponent::highlightColourId):Colours::darkgrey.darker());
			g.fillRoundedRectangle(hborder, hborder, filenameWidth-3.f-hborder, height-2.f*hborder, roundness);
		}

		{
			g.setGradientFill (juce::ColourGradient(juce::Colours::darkgrey,
											   0.f, height/2.f-hborder,
											   juce::Colour (0x8000),
											   0.7f*filenameWidth-3.f, height/2.f-hborder,
											   false));
			//g.setColour (Colours::darkgrey);
			g.drawRoundedRectangle(hborder, hborder, filenameWidth-3.f-hborder, height-2.f*hborder, roundness, 2.f);
		}
	
		const int iconhborder = height/2;
		const int x = height;
		const int y = height;
		g.setColour (juce::Colours::black);

	
		juce::Font f = g.getCurrentFont().withHeight(fontSize);
		f.setTypefaceName(/*"Forgotten Futurist Shadow"*/"Times New Roman");
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);

		g.setColour (findColour (juce::DirectoryContentsDisplayComponent::textColourId));

	
		int xText = x + 2*iconhborder;
		g.drawFittedText (name,
							xText, 0, width - xText, height,
							juce::Justification::centredLeft, 
							1, //1 line
							1.f//no h scale
							);
	}
	void drawFileBrowserRow (juce::Graphics& g, int width, int height,
										  const juce::String& filename, juce::Image* icon,
										  const juce::String& fileSizeDescription,
										  const juce::String& fileTimeDescription,
										  bool isDirectory,
										  bool isItemSelected,
										  int /*itemIndex*/,
										  juce::DirectoryContentsDisplayComponent&)
	{
		float fontSize =  0.9f*height;
		const int filenameWidth = width;//width > 450 ? roundToInt (width * 0.7f) : width;
	
		float hborder = height/8.f;
		float roundness = height/2.f;
	
	
	
		if(!isDirectory || isItemSelected)
		{
			g.setGradientFill (juce::ColourGradient (isItemSelected?findColour (juce::DirectoryContentsDisplayComponent::highlightColourId):juce::Colours::darkgrey.darker(),
											   0.f, height/2.f-hborder,
											   juce::Colour (0x8000),
											   0.7f*filenameWidth-3.f, height/2.f-hborder,
											   false));

			//g.setColour (isItemSelected?findColour (DirectoryContentsDisplayComponent::highlightColourId):Colours::darkgrey.darker());
			g.fillRoundedRectangle(hborder, hborder, filenameWidth-3.f-hborder, height-2.f*hborder, roundness);
		}

		if(!isDirectory)
		{
			g.setGradientFill (juce::ColourGradient(juce::Colours::darkgrey,
											   0.f, height/2.f-hborder,
											   juce::Colour (0x8000),
											   0.7f*filenameWidth-3.f, height/2.f-hborder,
											   false));
			//g.setColour (Colours::darkgrey);
			g.drawRoundedRectangle(hborder, hborder, filenameWidth-3-hborder, height-2.f*hborder, roundness, 2.f);
		}
	
		const int iconhborder = height/2;
		const int x = height;
		const int y = height;
		g.setColour (juce::Colours::black);

		if (icon != nullptr && icon->isValid() && !isDirectory)
		{
			g.drawImageWithin (*icon, 2 + iconhborder, 2, x-4, y-4,
							   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize,
							   false);
		}
		else if(isDirectory)
		{
			const juce::Drawable* d = isDirectory ? getDefaultFolderImage()
											: getDefaultDocumentFileImage();

			if (d != nullptr)
				d->drawWithin (g, juce::Rectangle<float> (2.0f  + iconhborder, 2.0f, x - 4.0f, y - 4.0f),
							   juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
		}
	
		juce::Font f = g.getCurrentFont().withHeight(fontSize);
		f.setTypefaceName(/*"Forgotten Futurist Shadow"*/"Times New Roman");
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);

		g.setColour (findColour (juce::DirectoryContentsDisplayComponent::textColourId));

	
		int xText = x + 2*iconhborder;
		g.drawFittedText (filename,
							xText, 0, width - xText, height,
							juce::Justification::centredLeft, 
							1, //1 line
							1.f//no h scale
							);
	}
};


#endif