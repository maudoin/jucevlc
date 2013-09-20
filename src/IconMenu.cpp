
#include "IconMenu.h"
#include <math.h>
#include "FileSorter.h"
#include "Icons.h"
#include <boost/format.hpp>
#include <boost/regex.hpp>

#define thunmnailW 300
#define thunmnailH 200
#define thumbnailCount 1
const int IconMenu::InvalidIndex = -1;

IconMenu::IconMenu()
	:m_mediaPostersXCount(5)
	,m_mediaPostersYCount(2)
	,m_leftArrowHighlighted(false)
	,m_rightArrowHighlighted(false)
	,m_sliderHighlighted(false)
	,img(new juce::Image(juce::Image::RGB, thunmnailW, thunmnailH*thumbnailCount, false))
	,ptr(new juce::Image::BitmapData(*img, juce::Image::BitmapData::readWrite))
{
    appImage = juce::ImageFileFormat::loadFrom(vlc_png, vlc_pngSize);
    folderImage = juce::Drawable::createFromImageData (folder_svg, folder_svgSize);
    upImage = juce::Drawable::createFromImageData (back_svg, back_svgSize);

	
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
	vlc = new VLCWrapper();
	vlc->Mute();
	vlc->SetBufferFormat(img->getWidth(), img->getHeight(), ptr->lineStride);
	vlc->SetDisplayCallback(this);
	vlc->SetEventCallBack(this);

}

IconMenu::~IconMenu()
{
	//let vlc threads finish
	thumbTimeOK = true;
	currentThumbnailIndex=thumbnailCount;
	currentThumbnail = juce::File::nonexistent;

	const juce::GenericScopedLock<juce::CriticalSection> lock (imgCriticalSection);
	vlc->Stop();
	vlc->clearPlayList();
	while(vlc->getCurrentPlayList().size()>0 ||vlc->isPlaying() )
	{
		//busy 
	}
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
		const juce::ScopedLock myScopedLock (m_imagesMutex);
		std::map<std::string, juce::Image>::const_iterator it = m_iconPerFile.find(file.getFileName().toUTF8().getAddress());
		if(it != m_iconPerFile.end())
		{
			image = it->second.isNull()?appImage:it->second;
		}
		else
		{
			image = appImage;
		}
	}
	else
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

	if(file==currentThumbnail)
	{
		g.setColour(juce::Colours::red);
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
		const int holeCount =  std::floor(rectWithBorders.getHeight()/(2.f*holeH));
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
		g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha(0.66f),
											x+itemW/2.f, rect.getBottom(),
											juce::Colours::purple.withAlpha(1.0f),
											x+itemW/2.f, rectWithBorders.getBottom(),
											false));
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

#include <iostream>

#define THUMB_TIME_POS 0.1
#define THUMB_TIME_POS_ERROR 0.9
bool IconMenu::storeImageInCache(juce::File const& f, juce::Image const& i)
{
	if(i.isNull())
	{
		{
			const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
			currentThumbnail = f;
			thumbTimeOK = false;
			currentThumbnailIndex = 0;
		}
		vlc->addPlayListItem(f.getFullPathName().toUTF8().getAddress());
		vlc->SetTime(0);
		vlc->play();
		return true;
	}
	const juce::ScopedLock myScopedLock (m_imagesMutex);
	std::cerr << f.getFileName().toUTF8().getAddress() << std::endl;
	m_iconPerFile.insert(std::map<std::string, juce::Image>::value_type(f.getFileName().toUTF8().getAddress(), i));
	return true;
}
void IconMenu::vlcTimeChanged()
{
	const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
	thumbTimeOK = (vlc->GetTime() > vlc->GetLength()*currentThumbnailIndex*THUMB_TIME_POS*THUMB_TIME_POS_ERROR);//90%margin
}
bool IconMenu::updatePreviews()
{
	if(vlc->getCurrentPlayList().size()>0 || vlc->isPlaying())
	{
		bool done;
		{
			const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
			done = currentThumbnail == juce::File::nonexistent;
		}
		if(done)
		{
			//stop outside vlc callbacks
			vlc->clearPlayList();
			vlc->Stop();
			return true;
		}
		else
		{
			const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
			if(vlc->GetTime() < vlc->GetLength()*currentThumbnailIndex*THUMB_TIME_POS*THUMB_TIME_POS_ERROR)//90%margin
			{
				vlc->SetTime((int64_t)(vlc->GetLength()*THUMB_TIME_POS*currentThumbnailIndex));
				thumbTimeOK = false;
			}
			//could process network code and stack another media though...
			return false;
		}
	}

	juce::Array<juce::File> files;
	{
		const juce::ScopedLock myScopedLock (m_imagesMutex);
		files = m_currentFiles;
	}
	juce::File file ;
	juce::String movieName ;
	for(juce::File* it = files.begin();it != files.end();++it)
	{
		if(it->isDirectory())
		{
			continue;
		}
		const juce::ScopedLock myScopedLock (m_imagesMutex);
		std::map<std::string, juce::Image>::const_iterator itImage = m_iconPerFile.find(it->getFileName().toUTF8().getAddress());
		if(itImage == m_iconPerFile.end())
		{
			file = *it;
			movieName = it->getFileNameWithoutExtension();
			break;
		}
	}
	if(movieName.isEmpty())
	{
		//all cache already fully setup
		return false;
	}
	movieName = movieName.replace("%", "%37");
	movieName = movieName.replace(" ", "%20");
	movieName = movieName.replace("_", "%20");
	movieName = movieName.replace(".", "%20");
	movieName = movieName.replace("é", "e");
	movieName = movieName.replace("è", "e");
	movieName = movieName.replace("ô", "o");
	movieName = movieName.replace("à", "a");
	std::string name = str( boost::format("http://www.omdbapi.com/?i=&t=%s")%std::string(movieName.toUTF8().getAddress()) );
	juce::URL url(name.c_str());
	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 1000, 0));
	if(!pIStream.get())
	{
		return storeImageInCache(file);
	}
	juce::MemoryOutputStream memStream(1000);//1ko at least
	if(memStream.writeFromInputStream(*pIStream, 100000)<=0)//100ko max
	{
		return storeImageInCache(file);
	}

	std::string ex("\"Poster\":\"([^\"]*)");
	boost::regex expression(ex, boost::regex::icase); 
	
	memStream.writeByte(0);//simulate end of c string
	boost::cmatch matches; 
	if(!boost::regex_search((char*)memStream.getData(), matches, expression)) 
	{
		return storeImageInCache(file);
	}
	juce::URL urlPoster(matches[1].str().c_str());
	juce::ScopedPointer<juce::InputStream> pIStreamImage(urlPoster.createInputStream(false, 0, 0, "", 1000, 0));//1 sec timeout
	if(!pIStreamImage.get())
	{
		return storeImageInCache(file);
	}
	return storeImageInCache(file, juce::ImageFileFormat::loadFrom (*pIStreamImage));
}


void *IconMenu::vlcLock(void **p_pixels)
{
	imgCriticalSection.enter();
	if(ptr)
	{
		*p_pixels = ptr->getLinePointer(currentThumbnailIndex>=thumbnailCount?0:(currentThumbnailIndex*thunmnailH));
	}
	if(thumbTimeOK)
	{
		currentThumbnailIndex++;
	}
	return NULL; /* picture identifier, not needed here */
}

void IconMenu::vlcUnlock(void *id, void *const *p_pixels)
{
	
	juce::Image copy = img->createCopy();
	imgCriticalSection.exit();
			
	juce::File processedThumbnail;
	int processedThumbnailIndex;
	{
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		processedThumbnail = currentThumbnail;
		processedThumbnailIndex = currentThumbnailIndex;
	}
	if( processedThumbnailIndex>=thumbnailCount)
	{
		storeImageInCache(processedThumbnail, copy);
		const juce::GenericScopedLock<juce::CriticalSection> lock (imgStatusCriticalSection);
		currentThumbnail = juce::File::nonexistent;
	}
	jassert(id == NULL); /* picture identifier, not needed here */
}

void IconMenu::vlcDisplay(void *id)
{
	jassert(id == NULL);
}