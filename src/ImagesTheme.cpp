
#include "ImagesTheme.h"

#include "Icons.h"

#include "AppConfig.h"
#include "juce.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

bool DrawableShapeNOP(juce::DrawableShape& d){return true;}
bool DrawableCompositeNOP(juce::DrawableComposite& d){return true;}

class DrawableRecursiveProcessor
{
public:
	typedef boost::function<bool (juce::DrawableShape&)> DrawableShapeProcessor;
	typedef boost::function<bool (juce::DrawableComposite&)> DrawableCompositeProcessor;
private:
    DrawableShapeProcessor         processShape;
    DrawableCompositeProcessor         processComposit;
public:

    DrawableRecursiveProcessor(DrawableShapeProcessor cbs, DrawableCompositeProcessor cbc=boost::bind(&DrawableCompositeNOP, _1))
    :processShape(cbs)
    ,processComposit(cbc)
    {
    }


    bool process(juce::Drawable* d)
    {
        bool proceed = true;
        if (!d)
        {
			return proceed;
		}

        juce::DrawableShape* ds = dynamic_cast<juce::DrawableShape*>(d);
        if (ds)
		{
            proceed = processShape(*ds);
		}
		else
		{
            juce::DrawableComposite* dc = dynamic_cast<juce::DrawableComposite*>(d);
            if (dc)
			{
                proceed = processComposit(*dc);
			}
        }

        int n = d->getNumChildComponents();
        for (int i = 0; proceed && i < n; ++i)
        {
            proceed = process(dynamic_cast<juce::Drawable*>(d->getChildComponent(i)));
        }
        return proceed;
    }
	static juce::Drawable* process(juce::Drawable* d,DrawableShapeProcessor cbs, DrawableCompositeProcessor cbc=boost::bind(&DrawableCompositeNOP, _1))
	{
		DrawableRecursiveProcessor p(cbs, cbc);
		p.process(d);
		return d;
	}
	
    
};

juce::DrawableShape::RelativeFillType changeSaturation(juce::DrawableShape::RelativeFillType const& fill, float newSaturation)
{
	juce::DrawableShape::RelativeFillType f = fill;
	if(f.fill.gradient)
	{
		for(int i=0;i<f.fill.gradient->getNumColours();++i)
		{
			f.fill.gradient->setColour(i, f.fill.gradient->getColour(i).withSaturation(newSaturation));
		}
	}
	f.fill.colour = f.fill.colour.withSaturation(newSaturation);

	return f;
}
juce::DrawableShape::RelativeFillType changeHue(juce::DrawableShape::RelativeFillType const& fill, float newHue)
{
	if(newHue>1.f)
	{
		return changeSaturation(fill, 0);
	}
	juce::DrawableShape::RelativeFillType f = fill;
	if(f.fill.gradient)
	{
		for(int i=0;i<f.fill.gradient->getNumColours();++i)
		{
			f.fill.gradient->setColour(i, f.fill.gradient->getColour(i).withHue(newHue));
		}
	}
	f.fill.colour = f.fill.colour.withHue(newHue);

	return f;
}

bool ChangeShapeHue(juce::DrawableShape& d, juce::String const& shapeName, float newHue)
{
	if(newHue < 0.f)
	{
		//no hue change, do not proceed further
		return false;
	}
	if(d.getName() == shapeName)
	{
		d.setFill(changeHue(d.getFill(), newHue));
		d.setStrokeFill(changeHue(d.getStrokeFill(), newHue));
		//found, do not proceed further
		return false;
	}

	//proceed to next shape
	return true;
}

inline juce::Drawable* applyTheme(juce::String const& shapeName, float newHue, juce::Drawable* d)
{
	return DrawableRecursiveProcessor::process(d, boost::bind(&ChangeShapeHue, _1, boost::ref(shapeName), newHue));
}
juce::Drawable* buildFolderBackImage(float hue)
{
	return applyTheme("rect1061", hue, juce::Drawable::createFromImageData (verticalFolderback_svg, verticalFolderback_svgSize));
}
juce::Drawable* buildFolderFrontImage(float hue)
{
	return applyTheme("rect1063", hue, juce::Drawable::createFromImageData (verticalFolderfront_svg, verticalFolderfront_svgSize));
}

juce::Drawable* buildBackImage(float hue)
{
	return applyTheme("path15341", hue, applyTheme("path15742", hue, juce::Drawable::createFromImageData (back_svg, back_svgSize)));
}


juce::Colour getThemeBaseColor(float hue)
{
	if(hue<0.)
	{
		return juce::Colours::purple;
	}
	else if(hue>1.)
	{
		return juce::Colours::darkgrey;
	}
	else
	{
		return juce::Colours::purple.withHue(hue);
	}
}