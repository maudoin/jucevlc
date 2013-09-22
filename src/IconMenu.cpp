
#include "IconMenu.h"
#include <math.h>
#include "FileSorter.h"
#include "Icons.h"
#include "PosterFinder.h"
#include <boost/format.hpp>
#include <boost/regex.hpp>

const int IconMenu::InvalidIndex = -1;

IconMenu::IconMenu()
	:m_mediaPostersXCount(5)
	,m_mediaPostersYCount(2)
	,m_leftArrowHighlighted(false)
	,m_rightArrowHighlighted(false)
	,m_sliderHighlighted(false)
	,m_thumbnailer(m_imageCatalog)
{
    appImage = juce::ImageFileFormat::loadFrom(vlc_png, vlc_pngSize);
    folderImage = juce::Drawable::createFromImageData (folder_svg, folder_svgSize);
    upImage = juce::Drawable::createFromImageData (back_svg, back_svgSize);

	

}

IconMenu::~IconMenu()
{
}
	
juce::Rectangle<float> IconMenu::getButtonAt(int index, float w, float h)
{	
	//non spaced size
	float itemW = w/m_mediaPostersXCount;
	float itemH = h/m_mediaPostersYCount; 

	//inside spacing
	float spaceX = itemW*0.1f;
	float spaceY = itemH*0.2f;

	//spaced size
	itemW -= 2*spaceX;
	itemH -= 2*spaceY;

	//internal space->screen space
	spaceX = (w-m_mediaPostersXCount*itemW)/(m_mediaPostersXCount+1);
	spaceY = (h-m_mediaPostersYCount*itemH)/(m_mediaPostersYCount+1);

	float x = spaceX/2.f + (itemW+spaceX) * (float)floor((float)index/m_mediaPostersYCount) ;
	float y = spaceY/2.f + (itemH+spaceY) * (index%m_mediaPostersYCount);
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
		setMediaStartIndex(index-(index%m_mediaPostersYCount));
		return true;
	}
	else if(leftRect.contains(xPos, yPos))
	{
		scrollDown();
		return true;
	}
	else if(rightRect.contains(xPos, yPos))
	{
		scrollUp();
		return true;
	}

	return false;
}

void IconMenu::scrollDown()
{
	const juce::ScopedLock myScopedLock (m_mutex);
	setMediaStartIndex(m_mediaPostersStartIndex - m_mediaPostersYCount*m_mediaPostersXCount);
}
void IconMenu::scrollUp()
{
	const juce::ScopedLock myScopedLock (m_mutex);
	setMediaStartIndex(m_mediaPostersStartIndex + m_mediaPostersYCount*m_mediaPostersXCount);
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
void IconMenu::paintMenu(juce::Graphics& g, float w, float h)
{
	juce::Rectangle<float> sliderRect = computeSliderRect(w, h);
	int countPerPage=m_mediaPostersXCount*m_mediaPostersYCount;
	int count=mediaCount();
	for(int i=0;i<std::min(count,countPerPage);i++)
	{
		paintItem(g,  i,  w, sliderRect.getY());
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

void IconMenu::paintItem(juce::Graphics& g, int index, float w, float h)
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

	juce::Image image;
	if(!file.isDirectory())
	{
		image = m_imageCatalog.get(file);
	}
	if(image.isNull())
	{
		image = appImage;
	}
	
	juce::Rectangle<float> firstRect = getButtonAt(0, w, h);
	float spaceX = firstRect.getX();
	float spaceY = firstRect.getY();
	
	float lineThickness = 1.5f;
        
	g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
	
	float sideStripWidth = 2.f*spaceX/3.f;		
	float holeW = 2.f*sideStripWidth/3.f;
	float holeH = holeW;	
	juce::Rectangle<float> rectWithBorders(x-sideStripWidth, y-holeH, itemW+2.f*sideStripWidth, itemH+spaceY+2.f*holeH);
	
	float titleHeight = rectWithBorders.getBottom()-rect.getBottom();

	if(index == m_mediaPostersHightlight)
	{
		g.setColour(juce::Colours::purple);
		g.fillRect(rectWithBorders);
	}

	if(m_thumbnailer.busyOn(file))
	{
		g.setColour(juce::Colours::purple.brighter());
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
	
		const float holeRoundness = holeW/6.f;
		const float holeBorderW = (sideStripWidth-holeW)/2.f;
		const int holeCount =  (int)std::floor(rectWithBorders.getHeight()/(2.f*holeH));
		const int holeBorderCount =  holeCount;
		const float holeBorderH = (rectWithBorders.getHeight() - holeCount*holeH)/holeBorderCount;
		for(float ih = rectWithBorders.getY();ih<=(rectWithBorders.getBottom()-holeBorderH);ih+=(holeH+holeBorderH))
		{
			g.fillRoundedRectangle(rectWithBorders.getX()+holeBorderW, ih, holeW, holeH, holeRoundness);
			g.fillRoundedRectangle(rect.getRight()+holeBorderW, ih, holeW, holeH, holeRoundness);
		}
	
		g.drawLine(rectWithBorders.getX(), rectWithBorders.getY(), rectWithBorders.getX(), rectWithBorders.getBottom(), lineThickness);
		g.drawLine(rectWithBorders.getRight(), rectWithBorders.getY(), rectWithBorders.getRight(), rectWithBorders.getBottom(), lineThickness);
			
		//picture
		//float reflectionScale = 0.3f;
		float imgHeightRatio = (float)image.getHeight()/**(1.f+reflectionScale)*//(float)image.getWidth();
		float itemHeightRatio = itemH/itemW;
		float scale;
		float imageX;
		float imageY;
		if(imgHeightRatio > itemHeightRatio)
		{
			//scale on h
			scale = itemH/(image.getHeight()/**(1.f+reflectionScale)*/);
			imageX = x + (itemW-image.getWidth()*scale)/2;
			imageY = rect.getY();
		}
		else
		{
			//scale on w
			scale = itemW/image.getWidth();
			imageX = x;
			imageY = rect.getBottom() - image.getHeight()*scale/**(1.f+reflectionScale)*/;
		}
		float imageItemW = image.getWidth()*scale;
		float imageItemH = image.getHeight()*scale;

		juce::AffineTransform tr = juce::AffineTransform::identity.scaled(scale, scale).translated(imageX, imageY);
		g.drawImageTransformed(image, tr, false);


		//reflect
		float reflectionH = titleHeight;//imageItemH*reflectionScale;
		juce::AffineTransform t = juce::AffineTransform::identity.scaled(scale, -/*scale*reflectionScale*/titleHeight/image.getHeight()).translated(imageX, rectWithBorders.getBottom());
		g.drawImageTransformed(image, t, false);

		//reflection floor
		juce::ColourGradient grad (juce::Colours::purple.withAlpha(0.33f),
											x+itemW/2.f, rect.getBottom(),
											juce::Colours::black.withAlpha(1.0f),
											x+itemW/2.f, rectWithBorders.getBottom(),
											false);
		
		grad.addColour(0.66, juce::Colours::purple.withAlpha(.66f));
		g.setGradientFill (grad);	
		g.fillRect(x, rect.getBottom(), itemW, reflectionH);
	
		//border
		g.setColour(juce::Colour(255, 255, 255));
		g.drawRect(rect, lineThickness);
	}


	//title
	const int maxTitleLineCount = 3;
	juce::Font f = g.getCurrentFont().withHeight(titleHeight/maxTitleLineCount);//2 lines max
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);
	g.setColour (juce::Colours::white);
	g.drawFittedText(file.exists()?isUpIcon?file.getParentDirectory().getFullPathName():file.getFileNameWithoutExtension():juce::String::empty,
		(int)(x),  (int)rect.getBottom(), 
		(int)(itemW), (int)(titleHeight), 
		juce::Justification::centredBottom, maxTitleLineCount, 1.f);

}

bool IconMenu::updatePreviews()
{
	if(m_thumbnailer.workStep())
	{
		//generation updated, refresh now
		return true;
	}

	juce::Array<juce::File> files;
	{
		const juce::ScopedLock myScopedLock (m_mutex);
		files = m_currentFiles;
	}
	for(juce::File* it = files.begin();it != files.end();++it)
	{
		if(it->isDirectory())
		{
			continue;
		}
		if(!m_imageCatalog.contains(*it))
		{
			//process this one
			juce::File &file = *it;
			juce::Image poster = PosterFinder::findPoster(file);
			if(!poster.isNull())
			{
				m_imageCatalog.storeImageInCache(file, poster);
				return true;
			}
			return m_thumbnailer.startGeneration(file);
		}
	}
	//nothing to do
	return false;

}

