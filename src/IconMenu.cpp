
#include "IconMenu.h"
#include <math.h>
#include "FileSorter.h"
#include "Icons.h"


const int IconMenu::InvalidIndex = -1;

IconMenu::IconMenu()
	:m_mediaPostersXCount(5)
	,m_mediaPostersYCount(2)
	,m_leftArrowHighlighted(false)
	,m_rightArrowHighlighted(false)
	,m_sliderHighlighted(false)
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
	return InvalidIndex;
}
bool IconMenu::clickOrDrag(float xPos, float yPos, float w, float h)
{
	juce::Rectangle<float> sliderRect = computeSliderRect(w, h);
	juce::Rectangle<float> leftRect = computeLeftArrowRect(sliderRect);
	juce::Rectangle<float> rightRect = computeRightArrowRect(sliderRect);
	if(sliderRect.contains(xPos, yPos))
	{
		int count=mediaCount();
		int index = (int)( count * (xPos - sliderRect.getX()) / sliderRect.getWidth() ) - m_mediaPostersXCount*m_mediaPostersYCount / 2;
		setMediaStartIndex(index);
		return true;
	}
	else if(leftRect.contains(xPos, yPos))
	{
		const juce::ScopedLock myScopedLock (m_mutex);
		setMediaStartIndex(m_mediaPostersStartIndex - m_mediaPostersYCount);
		return true;
	}
	else if(rightRect.contains(xPos, yPos))
	{
		const juce::ScopedLock myScopedLock (m_mutex);
		setMediaStartIndex(m_mediaPostersStartIndex + m_mediaPostersYCount);
		return true;
	}

	return false;
}
std::string IconMenu::getMediaAt(float xPos, float yPos, float w, float h)
{
	return getMediaAt(getButtonIndexAt(xPos, yPos, w, h));
}
void IconMenu::setMediaRootPath(std::string const& path)
{
    const juce::ScopedLock myScopedLock (m_mutex);
	m_mediaPostersAbsoluteRoot = path;
	setCurrentMediaRootPath(path);
}
void IconMenu::setCurrentMediaRootPath(std::string const& path)
{
    const juce::ScopedLock myScopedLock (m_mutex);
	m_mediaPostersRoot=path;

	juce::File file(m_mediaPostersRoot.c_str());
	
	juce::Array<juce::File> files;
	if(m_mediaPostersRoot.empty() ||!file.exists())
	{
		juce::File::findFileSystemRoots(files);
	}
	else
	{
		file.findChildFiles(files, juce::File::findFilesAndDirectories|juce::File::ignoreHiddenFiles, false, "*");
	}
	
	m_currentFiles.clear();
	for(juce::File* it = files.begin();it != files.end();++it)
	{
		if(extensionMatch(m_videoExtensions, it->getFileExtension()) || it->isDirectory())
		{
			m_currentFiles.add(*it);
		}
	}
	m_currentFiles.sort(FileSorter(m_videoExtensions));
	setMediaStartIndex(0);
}
void IconMenu::setMediaStartIndex(int index)
{
    const juce::ScopedLock myScopedLock (m_mutex);
	
	int count=mediaCount();
	int countPerPage=m_mediaPostersXCount*m_mediaPostersYCount;

	if(count <= countPerPage)
	{
		m_mediaPostersStartIndex = 0;
		return;
	}

	int max = count-countPerPage;
	m_mediaPostersStartIndex = index<0?0:(index>max?max:index);
	
}
std::string IconMenu::getMediaAt(int index)
{
	juce::File f=getMediaFileAt(index);
	return f.exists()?f.getFullPathName().toUTF8().getAddress():std::string();
}
juce::File IconMenu::getMediaFileAt(int indexOnScreen)
{
	if(IconMenu::InvalidIndex == indexOnScreen)
	{
		return juce::File();
	}

    const juce::ScopedLock myScopedLock (m_mutex);
	bool hasUpItem = m_mediaPostersAbsoluteRoot != m_mediaPostersRoot;
	if( hasUpItem && indexOnScreen==0)
	{
		juce::File f(m_mediaPostersRoot.c_str());
		return f.getParentDirectory();
	}
	
	int indexInfolder = indexOnScreen + m_mediaPostersStartIndex - (hasUpItem?1:0);
	if(indexInfolder>=0 && indexInfolder<m_currentFiles.size())
	{
		return m_currentFiles[indexInfolder];
	}
	return juce::File();
}
bool IconMenu::highlight(float xPos, float yPos, float w, float h)
{
	juce::Rectangle<float>sliderRect = computeSliderRect(w, h);

	int oldIndex = m_mediaPostersHightlight;
	bool oldLeft = m_leftArrowHighlighted;
	bool oldRight = m_rightArrowHighlighted;
	bool oldSlider = m_sliderHighlighted;

	m_mediaPostersHightlight = getButtonIndexAt(xPos, yPos, w, h);
	m_sliderHighlighted = sliderRect.contains(xPos, yPos);
	m_leftArrowHighlighted = computeLeftArrowRect(sliderRect).contains(xPos, yPos);
	m_rightArrowHighlighted = computeRightArrowRect(sliderRect).contains(xPos, yPos);

	return oldIndex != m_mediaPostersHightlight ||oldLeft != m_leftArrowHighlighted ||oldRight != m_rightArrowHighlighted || oldSlider != m_sliderHighlighted;
}

juce::Rectangle<float> IconMenu::computeSliderRect(float w, float h) const
{
	const float sliderRelativePos = 0.96f;
	float sliderTop = sliderRelativePos*h;
	float arrowW = h-sliderTop;
	return juce::Rectangle<float>(arrowW, sliderTop, w-2.f*arrowW, arrowW);
}
juce::Rectangle<float> IconMenu::computeLeftArrowRect(juce::Rectangle<float> const& slider) const
{
	return juce::Rectangle<float>(0, slider.getY(), slider.getX(), slider.getHeight());
}
juce::Rectangle<float> IconMenu::computeRightArrowRect(juce::Rectangle<float> const& slider) const
{
	return juce::Rectangle<float>(slider.getX()+slider.getWidth(), slider.getY(), slider.getX(), slider.getHeight());
}

int IconMenu::mediaCount()
{
	const juce::ScopedLock myScopedLock (m_mutex);
	
	return m_currentFiles.size() + (m_mediaPostersAbsoluteRoot != m_mediaPostersRoot?1:0);
}
void IconMenu::paintMenu(juce::Graphics& g, juce::Image const & appImage, float w, float h)
{
	juce::Rectangle<float> sliderRect = computeSliderRect(w, h);
	int countPerPage=m_mediaPostersXCount*m_mediaPostersYCount;
	int count=mediaCount();
	for(int i=0;i<std::min(count,countPerPage);i++)
	{
		paintItem(g, appImage, i,  w, sliderRect.getY());
	}
	

	if(count <= countPerPage)
	{
		return;
	}
	int firstMediaIndex=(m_mediaPostersStartIndex + countPerPage) > count ? count - countPerPage: m_mediaPostersStartIndex;
	float sliderStart = firstMediaIndex/(float)m_currentFiles.size();
	float sliderEnd = (firstMediaIndex+countPerPage)/(float)count;
	float sliderSize = sliderEnd-sliderStart;

	
	juce::ColourGradient gradient(juce::Colours::purple.brighter(),
										w/2.f, sliderRect.getHeight(),
										juce::Colours::purple.darker(),
										w/2.f, h,
										false);
	
	const float thick = 2.f;
	const float thin = 1.f;
	float thickness = m_sliderHighlighted?thick:thin;

	juce::Rectangle<float> total(sliderRect.getX(), sliderRect.getY()+sliderRect.getHeight()/4.f, sliderRect.getWidth(), sliderRect.getHeight()/2.f);
	g.setGradientFill(gradient);
	g.fillRect(total);
	g.setColour(juce::Colours::white);
	g.drawRect(total, thickness);
	
	float roundness = sliderRect.getHeight()/6.f;
	
	juce::Rectangle<float> current(sliderRect.getX()+sliderRect.getWidth()*sliderStart, sliderRect.getY(), sliderRect.getWidth()*sliderSize, sliderRect.getHeight());
	g.setGradientFill(gradient);
	g.fillRoundedRectangle(current, roundness);
	g.setColour(juce::Colours::white);
	g.drawRoundedRectangle(current, roundness, thickness);
	
	float arrowW = sliderRect.getX();
	float arrowY = sliderRect.getY()+sliderRect.getHeight()/2.f;
	juce::Path arrow;
	arrow.addArrow(juce::Line<float>(sliderRect.getX(), arrowY, 0 , arrowY),sliderRect.getHeight(), sliderRect.getHeight(), arrowW);
	g.fillPath(arrow);

	juce::Path arrowRight;
	arrowRight.addArrow(juce::Line<float>(w-sliderRect.getX(), arrowY, w, arrowY),sliderRect.getHeight(), sliderRect.getHeight(), arrowW);
	
	g.setColour(juce::Colours::purple);
	g.fillPath(arrowRight);
	g.fillPath(arrow);

	g.setColour(juce::Colours::white);
	thickness = m_rightArrowHighlighted?thick:thin;
	g.strokePath(arrowRight, juce::PathStrokeType(thickness));
	thickness = m_leftArrowHighlighted?thick:thin;
	g.strokePath(arrow, juce::PathStrokeType(thickness));
}

void IconMenu::paintItem(juce::Graphics& g, juce::Image const & i, int index, float w, float h)
{
	juce::File file=getMediaFileAt(index);
	if(!file.exists())
	{
		return;
	}

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

	
	bool isUpIcon = index == 0 && m_mediaPostersAbsoluteRoot != m_mediaPostersRoot;
	if(isUpIcon)
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
	g.drawFittedText(file.exists()?isUpIcon?file.getParentDirectory().getFullPathName():file.getFileNameWithoutExtension():juce::String::empty,
		(int)(x),  (int)(y+realItemH+reflectionH+holeH), 
		(int)(realItemW), (int)(3.f*holeH), 
		juce::Justification::centred, 3, 0.85f);

}