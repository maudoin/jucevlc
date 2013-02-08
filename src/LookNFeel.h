#ifndef LOOKANDFEEL
#define LOOKANDFEEL

#include "juce.h"
#include "FontSerialization.h"
#include "AppProportionnalComponent.h"


class LnF : public OldSchoolLookAndFeel
{
	Typeface* cFont;

public:
	LnF()
	{
		setColour(DirectoryContentsDisplayComponent::textColourId, Colours::white);
#ifdef _DEBUG
		serializeFont("Forgotten Futurist Shadow", "ForgottenFuturistShadow.bin.new");
#endif
		cFont = loadFont( "ForgottenFuturistShadow.bin");
	}
	const Typeface::Ptr getTypefaceForFont (const Font &font)
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
	
    virtual void drawLinearSlider (Graphics& g,
                                   int x, int y,
                                   int width, int height,
                                   float sliderPos,
                                   float minSliderPos,
                                   float maxSliderPos,
                                   const Slider::SliderStyle style,
                                   Slider& slider)
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
	void drawListRow (Graphics& g, int width, int height,const String& name,
										  bool isItemSelected)
	{
		double fontSize =  0.9*height;
		const int filenameWidth = width;//width > 450 ? roundToInt (width * 0.7f) : width;
	
		const int hborder = height/8;
		const int roundness = height/2;
	
	
	
		if(isItemSelected)
		{
			g.setGradientFill (ColourGradient (isItemSelected?findColour (DirectoryContentsDisplayComponent::highlightColourId):Colours::darkgrey.darker(),
											   0, height/2-hborder,
											   Colour (0x8000),
											   0.7*filenameWidth-3, height/2-hborder,
											   false));

			//g.setColour (isItemSelected?findColour (DirectoryContentsDisplayComponent::highlightColourId):Colours::darkgrey.darker());
			g.fillRoundedRectangle(hborder, hborder, filenameWidth-3-hborder, height-2*hborder, roundness);
		}

		{
			g.setGradientFill (ColourGradient(Colours::darkgrey,
											   0, height/2-hborder,
											   Colour (0x8000),
											   0.7*filenameWidth-3, height/2-hborder,
											   false));
			//g.setColour (Colours::darkgrey);
			g.drawRoundedRectangle(hborder, hborder, filenameWidth-3-hborder, height-2*hborder, roundness, 2);
		}
	
		const int iconhborder = height/2;
		const int x = height;
		const int y = height;
		g.setColour (Colours::black);

	
		Font f = g.getCurrentFont().withHeight(fontSize);
		f.setTypefaceName(/*"Forgotten Futurist Shadow"*/"Times New Roman");
		f.setStyleFlags(Font::FontStyleFlags::plain);
		g.setFont(f);

		g.setColour (findColour (DirectoryContentsDisplayComponent::textColourId));

	
		int xText = x + 2*iconhborder;
		g.drawFittedText (name,
							xText, 0, width - xText, height,
							Justification::centredLeft, 
							1, //1 line
							1.f//no h scale
							);
	}
	void drawFileBrowserRow (Graphics& g, int width, int height,
										  const String& filename, Image* icon,
										  const String& fileSizeDescription,
										  const String& fileTimeDescription,
										  const bool isDirectory,
										  const bool isItemSelected,
										  const int /*itemIndex*/,
										  DirectoryContentsDisplayComponent&)
	{
		double fontSize =  0.9*height;
		const int filenameWidth = width;//width > 450 ? roundToInt (width * 0.7f) : width;
	
		const int hborder = height/8;
		const int roundness = height/2;
	
	
	
		if(!isDirectory || isItemSelected)
		{
			g.setGradientFill (ColourGradient (isItemSelected?findColour (DirectoryContentsDisplayComponent::highlightColourId):Colours::darkgrey.darker(),
											   0, height/2-hborder,
											   Colour (0x8000),
											   0.7*filenameWidth-3, height/2-hborder,
											   false));

			//g.setColour (isItemSelected?findColour (DirectoryContentsDisplayComponent::highlightColourId):Colours::darkgrey.darker());
			g.fillRoundedRectangle(hborder, hborder, filenameWidth-3-hborder, height-2*hborder, roundness);
		}

		if(!isDirectory)
		{
			g.setGradientFill (ColourGradient(Colours::darkgrey,
											   0, height/2-hborder,
											   Colour (0x8000),
											   0.7*filenameWidth-3, height/2-hborder,
											   false));
			//g.setColour (Colours::darkgrey);
			g.drawRoundedRectangle(hborder, hborder, filenameWidth-3-hborder, height-2*hborder, roundness, 2);
		}
	
		const int iconhborder = height/2;
		const int x = height;
		const int y = height;
		g.setColour (Colours::black);

		if (icon != nullptr && icon->isValid() && !isDirectory)
		{
			g.drawImageWithin (*icon, 2 + iconhborder, 2, x-4, y-4,
							   RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
							   false);
		}
		else if(isDirectory)
		{
			const Drawable* d = isDirectory ? getDefaultFolderImage()
											: getDefaultDocumentFileImage();

			if (d != nullptr)
				d->drawWithin (g, Rectangle<float> (2.0f  + iconhborder, 2.0f, x - 4.0f, y - 4.0f),
							   RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
		}
	
		Font f = g.getCurrentFont().withHeight(fontSize);
		f.setTypefaceName(/*"Forgotten Futurist Shadow"*/"Times New Roman");
		f.setStyleFlags(Font::FontStyleFlags::plain);
		g.setFont(f);

		g.setColour (findColour (DirectoryContentsDisplayComponent::textColourId));

	
		int xText = x + 2*iconhborder;
		g.drawFittedText (filename,
							xText, 0, width - xText, height,
							Justification::centredLeft, 
							1, //1 line
							1.f//no h scale
							);
	}
};


#endif