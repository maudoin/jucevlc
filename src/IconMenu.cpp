
#include "IconMenu.h"
#include <math.h>
#include "FileSorter.h"
#include "Icons.h"


IconMenu::IconMenu()
	:m_mediaPostersXCount(5)
	,m_mediaPostersYCount(2)
{
    folderImage = juce::Drawable::createFromImageData (folder_svg, folder_svgSize);
    upImage = juce::Drawable::createFromImageData (back_svg, back_svgSize);
}

IconMenu::~IconMenu()
{
}
	
juce::Rectangle<float> IconMenu::getButtonAt(int index, float w, float h)
{	
	float itemW = w/(m_mediaPostersXCount+1);
	float itemH = h/(m_mediaPostersYCount+1);

	float spaceX = itemW/m_mediaPostersXCount;
	float spaceY = itemH/m_mediaPostersYCount;

	float x = spaceX/2.f + (itemW+spaceX) * (float)floor((float)index/m_mediaPostersYCount) ;
	float y = (itemH+spaceY) * (index%m_mediaPostersYCount);
	return juce::Rectangle<float>(x, y, itemW, itemH);
}
int IconMenu::getButtonIndexAt(float xPos, float yPos, float w, float h)
{
	for(int i=0;i<m_mediaPostersXCount*m_mediaPostersYCount;i++)
	{
		juce::Rectangle<float> rect = getButtonAt(i, w, h);
		if(rect.contains(xPos, yPos))
		{
			return i;
		}
		
	}
	return -1;
}
std::string IconMenu::clickOrGetMediaAt(float xPos, float yPos, float w, float h)
{
	//click arrows
	return getMediaAt(xPos, yPos, w, h);
}
std::string IconMenu::getMediaAt(float xPos, float yPos, float w, float h)
{
	return getMediaAt(getButtonIndexAt(xPos, yPos, w, h));
}
void IconMenu::setMediaStartIndex(int index)
{
    const juce::ScopedLock myScopedLock (m_mutex);

	m_mediaPostersStartIndex = index;
	m_currentFiles.clear();

	juce::File file(m_mediaPostersRoot.c_str());
	file.findChildFiles(m_currentFiles, juce::File::findFilesAndDirectories|juce::File::ignoreHiddenFiles, false, "*");
	m_currentFiles.sort(FileSorter(m_videoExtensions));
	
}
std::string IconMenu::getMediaAt(int index)
{
	juce::File f=getMediaFileAt(index);
	return f.exists()?f.getFullPathName().toUTF8().getAddress():std::string();
}
juce::File IconMenu::getMediaFileAt(int index)
{
    const juce::ScopedLock myScopedLock (m_mutex);
	if(index==0)
	{
		juce::File f(m_mediaPostersRoot.c_str());
		return f.getParentDirectory();
	}

	if(index>=0 && index<m_currentFiles.size())
	{
		return m_currentFiles[index];
	}
	return juce::File();
}
void IconMenu::highlight(float xPos, float yPos, float w, float h)
{
	m_mediaPostersHightlight = getButtonIndexAt(xPos, yPos, w, h);
}

void IconMenu::paintMenu(juce::Graphics& g, juce::Image const & appImage, float w, float h)
{
	float hList = 0.96f*h;
	for(int i=0;i<m_mediaPostersXCount*m_mediaPostersYCount;i++)
	{
		paintItem(g, appImage, i,  w, hList);
	}
	
	float sliderStart = m_mediaPostersStartIndex/(float)m_currentFiles.size();
	float sliderEnd = (m_mediaPostersStartIndex+m_mediaPostersXCount*m_mediaPostersYCount)/(float)m_currentFiles.size();
	float sliderSize = sliderEnd-sliderStart;

	float arrowH = h-hList;
	float arrowW = arrowH;
	
	juce::ColourGradient gradient(juce::Colours::purple.brighter(),
										w/2.f, hList,
										juce::Colours::purple.darker(),
										w/2.f, h,
										false);
	
	float sliderTotalScreenStart = arrowW;
	float sliderTotalScreenW = w-2.f*arrowW;;

	juce::Rectangle<float> total(sliderTotalScreenStart, hList+arrowH/4.f, sliderTotalScreenW, arrowH/2.f);
	g.setGradientFill(gradient);
	g.fillRect(total);
	g.setColour(juce::Colours::white);
	g.drawRect(total, 2.f);
	
	juce::Rectangle<float> current(sliderTotalScreenStart+sliderTotalScreenW*sliderStart, hList, sliderTotalScreenW*sliderSize, arrowH);
	g.setGradientFill(gradient);
	g.fillRoundedRectangle(current, arrowH/6.f);
	g.setColour(juce::Colours::white);
	g.drawRoundedRectangle(current, arrowH/6.f, 2.f);

	float arrowY = hList+arrowH/2.f;
	juce::Path arrow;
	arrow.addArrow(juce::Line<float>(arrowW, arrowY, 0 , arrowY),arrowH, arrowH, arrowW);
	g.fillPath(arrow);

	juce::Path arrowRight;
	arrowRight.addArrow(juce::Line<float>(w-arrowW, arrowY, w, arrowY),arrowH, arrowH, arrowW);
	
	g.setColour(juce::Colours::purple);
	g.fillPath(arrowRight);
	g.fillPath(arrow);

	g.setColour(juce::Colours::white);
	g.strokePath(arrowRight, juce::PathStrokeType(2.f));
	g.strokePath(arrow, juce::PathStrokeType(2.f));
}

void IconMenu::paintItem(juce::Graphics& g, juce::Image const & i, int index, float w, float h)
{
	juce::Rectangle<float> rect = getButtonAt(index, w, h);
	float itemW = rect.getWidth();
	float itemH = rect.getHeight();
	float x = rect.getX();
	float y = rect.getY();

	
	
	float spaceX = itemW/m_mediaPostersXCount;
	float spaceY = itemH/m_mediaPostersYCount;
	float scale = itemW/i.getWidth();
	float realItemW = itemW;
	float realItemH = itemH * scale / (itemH/i.getHeight());

	float reflectionScale = 0.3f;
	float reflectionH = realItemH*reflectionScale;


	
	float lineThickness = 1.5f;
        
	g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
	
		
	int holeCount = 15;
	float holeH = 0.5f*(realItemH+reflectionH)/(float)holeCount;
	float holeW = std::min(spaceX/2.f, holeH);
	
	if(index == m_mediaPostersHightlight)
	{
		g.setColour(juce::Colours::purple);
		g.fillRect(rect);
	}

	g.setOpacity (1.f);

	

	juce::File file=getMediaFileAt(index);
	if(index == 0)
	{
		upImage.get()->drawWithin (g, rect,
							juce::RectanglePlacement::centred | juce::RectanglePlacement::stretchToFit, 1.0f);
	}
	else if(file.isDirectory())
	{
		folderImage.get()->drawWithin (g, rect,
							juce::RectanglePlacement::centred | juce::RectanglePlacement::stretchToFit, 1.0f);
	}
	else
	{
		//border
		g.setColour(juce::Colour(255, 255, 255));
	
		float holesOriginY = y+holeH;
		for(int ih = 0;ih<holeCount;ih++)
		{
			g.fillRoundedRectangle(x-holeW-holeW/4.f, holesOriginY+ih*holeH*2.f, holeW, holeH, holeW/6.f);
			g.fillRoundedRectangle(x+realItemW+holeW/4.f, holesOriginY+ih*holeH*2.f, holeW, holeH, holeW/6.f);
		}
	
		float xLeft = x-1.5f*holeW;
		float xRight = x+realItemW+1.5f*holeW;
		g.drawLine(xLeft, y-2*holeH, xLeft, y + realItemH+reflectionH+4*holeH, lineThickness);
		g.drawLine(xRight, y-2*holeH, xRight, y + realItemH+reflectionH+4*holeH, lineThickness);

		//top
		/*
		float prevLineBottom = y-holeH;
		g.drawLine(x, y-2*holeH, x, prevLineBottom, lineThickness);
		g.drawLine(x+realItemW, y-2*holeH, x+realItemW, prevLineBottom, lineThickness);
		g.drawLine(x, prevLineBottom, x+realItemW, prevLineBottom, lineThickness);
		*/
	
		//picture
		juce::AffineTransform tr = juce::AffineTransform::identity.scaled(scale, scale).translated(x, y);
		g.drawImageTransformed(i, tr, false);


		//reflect
		juce::AffineTransform t = juce::AffineTransform::identity.scaled(scale, -scale*reflectionScale).translated(x, y+realItemH+reflectionH);
		g.drawImageTransformed(i, t, false);

		//reflection
		g.setGradientFill (juce::ColourGradient (juce::Colours::black,
											x+realItemW/2.f, y+realItemH,
											juce::Colours::purple.withAlpha(0.5f),
											x+realItemW/2.f, y+realItemH+reflectionH,
											false));
		g.fillRect(x, y+realItemH, realItemW, reflectionH);
	
		//border
		g.setColour(juce::Colour(255, 255, 255));
		g.drawRect(x, y, realItemW, realItemH+reflectionH, lineThickness);
	}


	//title
	float fontHeight = 2.f*holeH;
	juce::Font f = g.getCurrentFont().withHeight(fontHeight);
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);
	g.setColour (juce::Colours::grey);
	g.drawFittedText(juce::String(file.getFileNameWithoutExtension()),
		(int)(x),  (int)(y+realItemH+reflectionH+holeH), 
		(int)(realItemW), (int)(3.f*holeH), 
		juce::Justification::centred, 3, 0.85f);

}