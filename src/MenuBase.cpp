
#include "MenuBase.h"


MenuBase::MenuBase()
{
}

MenuBase::~MenuBase()
{
}
	
void MenuBase::paintMenuBackGround(juce::Graphics& g)
{
	float w = (float)asComponent()->getWidth();
	float h = (float)asComponent()->getHeight();
	float hMargin = 0.025f*asComponent()->getParentWidth();
	float roundness = hMargin/4.f;
	
	///////////////// TREE ZONE:	
	g.setGradientFill (juce::ColourGradient (juce::Colours::darkgrey.darker().withAlpha(0.75f),
										0, h/2.f,
										juce::Colours::black,
										w, h/2.f,
										false));
	g.fillRoundedRectangle(0, 0, w, h, roundness);

	g.setGradientFill (juce::ColourGradient (juce::Colours::grey.withAlpha(0.75f),
										0, h/2.f,
										juce::Colours::black,
										w, h/2.f,
										false));
	g.drawRoundedRectangle(0, 0, w, h, roundness,2.f);
	

}
//==============================================================================


void paintMenuItem (juce::Graphics& g, int width, int height, bool isItemSelected, juce::String const& name, const juce::Drawable* d, bool isShortcut)
{
		
	float fontSize =  1.1f*height;
	
	float hborder = height/8.f;	
	
	float iconhborder = height/4.f;
	int iconSize = height;

		
	float xBounds = iconSize + iconhborder;

	juce::Rectangle<float> borderBounds(xBounds, hborder, width-xBounds, height-2.f*hborder);

	if(isItemSelected)
	{
		g.setGradientFill (juce::ColourGradient(juce::Colours::blue.darker(),
											borderBounds.getX(), 0.f,
											juce::Colours::black,
											borderBounds.getRight(), 0.f,
											false));

		g.fillRect(borderBounds);

		g.setGradientFill (juce::ColourGradient(juce::Colours::blue.brighter(),
											borderBounds.getX(), 0.f,
											juce::Colours::black,
											borderBounds.getRight(), 0.f,
											false));

		g.drawRect(borderBounds);
	}

	if(!isShortcut)
	{
		g.setGradientFill (juce::ColourGradient(juce::Colours::lightgrey,
											borderBounds.getCentreX(), 0.f,
											juce::Colours::black.withAlpha(0.f),
											borderBounds.getRight(), 0.f,
											true));

		g.drawLine(0, 0, (float)width, 0, 2.f);
	}
	
	g.setColour (juce::Colours::black);
		
	if (d != nullptr)
	{
			d->drawWithin (g, juce::Rectangle<float> (iconhborder, 0.0f, (float)iconSize, (float)iconSize),
							juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
	}
	
	juce::Font f = g.getCurrentFont().withHeight(fontSize);
	f.setTypefaceName(/*"Forgotten Futurist Shadow"*/"Times New Roman");
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);

	g.setColour (juce::Colours::white);
		
	int xText = iconSize + 2*(int)iconhborder;
	g.drawFittedText (name,
						xText, 0, width - xText, height,
						juce::Justification::centredLeft, 
						1, //1 line
						1.f//no h scale
						);
}