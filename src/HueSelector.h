#ifndef HUE_SELECTOR_H
#define HUE_SELECTOR_H

#include "AppConfig.h"
#include "juce.h"

class HueSelectorMarker  : public juce::Component
{
	float relativeHeight;
public:
    HueSelectorMarker(float relativeHeight = 0.3f):relativeHeight(relativeHeight)
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (juce::Graphics& g) override
    {
        const float w = (float) getWidth();
        const float h = (float) getHeight();

        juce::Path p;
        p.addTriangle (1.0f, 1.0f,
                       w * 0.5f, h * relativeHeight,
                       w - 1.0f, 1.0f);

        p.addTriangle (1.0f, h - 1.0f,
                       w * 0.5f, h * (1.f-relativeHeight),
                       w - 1.0f, h - 1.0f);

        g.setColour (juce::Colours::white.withAlpha (0.75f));
        g.fillPath (p);

        g.setColour (juce::Colours::black.withAlpha (0.75f));
        g.strokePath (p, juce::PathStrokeType (1.2f));
    }

private:
    JUCE_DECLARE_NON_COPYABLE (HueSelectorMarker)
};

class HueSelectorComp  : public juce::Component
{
public:
    HueSelectorComp (float hue, const int edgeSize, float markerRelativeHeight = 0.3f)
        : h (hue), marker(markerRelativeHeight), edge (edgeSize)
    {
		updateIfNeeded();
        addAndMakeVisible (&marker);
    }

    void paint (juce::Graphics& g) override
    {
        juce::ColourGradient cg;
        cg.isRadial = false;
        cg.point1.setXY ((float) edge, 0.0f);
        cg.point2.setXY ((float) (getWidth() - edge), 0.0f);

        for (float i = 0.0f; i <= 1.0f; i += 0.02f)
            cg.addColour (i, juce::Colour (i, 1.0f, 1.0f, 1.0f));

        g.setGradientFill (cg);
        g.fillRect (getLocalBounds().reduced (edge));
    }

    void updateIfNeeded() 
    {
        marker.setBounds (juce::roundToInt ((getWidth() - edge * 2) * h), 0, edge * 2, getHeight());
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
		h = juce::jlimit (0.0f, 1.0f, (e.x - edge) / (float) (getWidth() - edge * 2));
		updateIfNeeded();
    }
	
    void resized() override
    {
        updateIfNeeded();
    }
    float getHue()
    {
        return h;
    }

private:
    float h;
    HueSelectorMarker marker;
    const int edge;

    JUCE_DECLARE_NON_COPYABLE (HueSelectorComp)
};

#endif //HUE_SELECTOR_H