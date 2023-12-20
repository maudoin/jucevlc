#ifndef LOOKANDFEEL
#define LOOKANDFEEL

#include <JuceHeader.h>
#include "AppProportionnalComponent.h"
#include "Fonts.h"


class LnF : public juce::LookAndFeel_V2
{
	juce::Typeface::Ptr cFont;

public:
	LnF()
	{
		setColour(juce::DirectoryContentsDisplayComponent::textColourId, juce::Colours::white);
		cFont = juce::Typeface::createSystemTypefaceFor(Fonts::NotoSansRegular_ttf, Fonts::NotoSansRegular_ttfSize);
	}
	juce::Typeface::Ptr getTypefaceForFont (const juce::Font &font) override
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

    void drawRoundThumb (Graphics& g, float x, float y, float diameter, Colour colour, float outlineThickness)
    {
        auto halfThickness = outlineThickness * 0.5f;

        Path p;
        p.addEllipse (x + halfThickness,
                      y + halfThickness,
                      diameter - outlineThickness,
                      diameter - outlineThickness);

        DropShadow (Colours::black, 1, {}).drawForPath (g, p);

        g.setColour (colour);
        g.fillPath (p);

        g.setColour (colour.brighter());
        g.strokePath (p, PathStrokeType (outlineThickness));
    }
    void drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                Slider::SliderStyle style, Slider& slider) override
    {
        auto sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

        auto isDownOrDragging = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());

        auto knobColour = slider.findColour (Slider::thumbColourId)
                                .withMultipliedSaturation ((slider.hasKeyboardFocus (false) || isDownOrDragging) ? 1.3f : 0.9f)
                                .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.7f);

        if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
        {
            float kx, ky;

            if (style == Slider::LinearVertical)
            {
                kx = (float) x + (float) width * 0.5f;
                ky = sliderPos;
            }
            else
            {
                kx = sliderPos;
                ky = (float) y + (float) height * 0.5f;
            }

            auto outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

            drawRoundThumb (g,
                            kx - sliderRadius,
                            ky - sliderRadius,
                            sliderRadius * 2.0f,
                            knobColour, outlineThickness);
        }
        else
        {
            // Just call the base class for the demo
            LookAndFeel_V2::drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           Slider::SliderStyle style, Slider& slider) override
    {
        g.fillAll (slider.findColour (Slider::backgroundColourId));

        if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
        {
            Path p;

            if (style == Slider::LinearBarVertical)
                p.addRectangle ((float) x, sliderPos, (float) width, 1.0f + (float) height - sliderPos);
            else
                p.addRectangle ((float) x, (float) y, sliderPos - (float) x, (float) height);

            auto baseColour = slider.findColour (Slider::rotarySliderFillColourId)
                                    .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                                    .withMultipliedAlpha (0.8f);

            g.setColour (baseColour);
            g.fillPath (p);

            auto lineThickness = jmin (15.0f, (float) jmin (width, height) * 0.45f) * 0.1f;
            g.drawRect (slider.getLocalBounds().toFloat(), lineThickness);
        }
        else
        {
            drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            drawLinearSliderThumb      (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

};


#endif