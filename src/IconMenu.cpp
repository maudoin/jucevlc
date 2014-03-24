
#include "IconMenu.h"
#include <math.h>
#include "FileSorter.h"
#include "Icons.h"
#include "PosterFinder.h"
#include <boost/format.hpp>
#include <boost/regex.hpp>

const int IconMenu::InvalidIndex = -1;
const int IconMenu::UpFolderIndex = -2;

#define MAX_THUMBNAILS_PRELOAD_TIME_MS 500
IconMenu::IconMenu()
	:m_mediaPostersXCount(6)
	,m_mediaPostersYCount(2)
	,m_leftArrowHighlighted(false)
	,m_rightArrowHighlighted(false)
	,m_sliderHighlighted(false)
	,m_thumbnailer(m_imageCatalog)
{
    appImage = juce::ImageFileFormat::loadFrom(vlc_png, vlc_pngSize);
    driveImage = juce::Drawable::createFromImageData (harddisk_svg, harddisk_svgSize);
    diskImage = juce::Drawable::createFromImageData (disk_svg, disk_svgSize);
    usbImage = juce::Drawable::createFromImageData (usb_svg, usb_svgSize);
	upImage  = juce::Drawable::createFromImageData (back_svg, back_svgSize);
}

IconMenu::~IconMenu()
{
}
	
juce::Rectangle<float> IconMenu::getButtonAt(int index, float w, float h)const
{	
	
	int i = (int)floor((float)index/m_mediaPostersYCount);
	int j = (index%m_mediaPostersYCount);

	juce::Rectangle<float> sliderRect = computeSliderRect(w, h);

	float menuW = w;
	float menuH = h-sliderRect.getHeight();

	float border = (menuW>menuH ? menuH : menuW)*0.025f;

	float spacingX = menuW*0.01f/(float)(m_mediaPostersXCount-1);
	float spacingY = menuH*0.01f/(float)(m_mediaPostersYCount-1);


	float itemW = (menuW-2.f*border-(m_mediaPostersXCount-1)*spacingX)/m_mediaPostersXCount;
	float itemH = (menuH-2.f*border-(m_mediaPostersYCount-1)*spacingY)/m_mediaPostersYCount; 
	
	float x = border + (itemW+spacingX) * i;
	float y = border + (itemH+spacingY) * j;

	return juce::Rectangle<float>(x, y, itemW, itemH);
}
int IconMenu::getButtonIndexAt(float xPos, float yPos, float w, float h)const
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
std::string IconMenu::getMediaAtIndexOnScreen(float xPos, float yPos, float w, float h)const
{
	return getMediaAtIndexOnScreen(getButtonIndexAt(xPos, yPos, w, h));
}
void IconMenu::setMediaRootPath(std::string const& path)
{
    const juce::ScopedLock myScopedLock (m_mutex);
	m_mediaPostersAbsoluteRoot = path;
	setCurrentMediaRootPath(path);
}

struct PathAndImageSorter : public FileSorter
{
	PathAndImageSorter(std::set<juce::String> const& priorityExtensions_):FileSorter(priorityExtensions_){}
	PathAndImageSorter(std::vector< std::set<juce::String> > const& priorityExtensions_):FileSorter(priorityExtensions_) {}
	int compareElements(IconMenu::PathAndImage const& some, IconMenu::PathAndImage const& other)
	{
		return FileSorter::compareElements(some.first, other.first);
	}
};

void IconMenu::setCurrentMediaRootPath(std::string const& path)
{
	std::string newRoot = path;

	juce::File file(juce::String::fromUTF8(newRoot.c_str()));
	if(!file.exists())
	{
		newRoot=std::string();
	}
	
	juce::Array<juce::File> files;
	if(newRoot.empty())
	{
		juce::File::findFileSystemRoots(files);
	}
	else
	{
		file.findChildFiles(files, juce::File::findFilesAndDirectories|juce::File::ignoreHiddenFiles, false, "*");
	}
	
	{
		const juce::ScopedLock myScopedLock (m_mutex);
		m_mediaPostersRoot=newRoot;
	}
	{
		const juce::ScopedLock myScopedLock (m_currentFilesMutex);
		m_currentFiles.clear();
		for(juce::File* it = files.begin();it != files.end();++it)
		{
			if(it->exists() && ( ::extensionMatch(m_videoExtensions, it->getFileExtension()) || it->isDirectory()) )
			{
				m_currentFiles.add(PathAndImage(*it, false));
			}
		}
		PathAndImageSorter sorter(m_videoExtensions);
		m_currentFiles.sort(sorter);

		m_imageCatalog.preload(m_currentFiles, MAX_THUMBNAILS_PRELOAD_TIME_MS);
	}

	

	setMediaStartIndex(0);
}
juce::File IconMenu::findFirstMovie(juce::File const& file)const
{
	
	juce::Array<juce::File> files;
	file.findChildFiles(files, juce::File::findFilesAndDirectories|juce::File::ignoreHiddenFiles, false, "*");
	FileSorter sorter(m_videoExtensions);
	files.sort(sorter);
	//try movies
	for(juce::File* it = files.begin();it != files.end();++it)
	{
		if(extensionMatch(m_videoExtensions, it->getFileExtension()))
		{
			return *it;
		}
	}
	//try folders
	for(juce::File* it = files.begin();it != files.end();++it)
	{
		if(it->isDirectory())
		{
			//recurse
			return findFirstMovie(*it);
		}
	}
	return juce::File();

}
void IconMenu::setMediaStartIndex(int index)
{
	
	int count=mediaCount();

    const juce::ScopedLock myScopedLock (m_mutex);
	int countPerPage=m_mediaPostersXCount*m_mediaPostersYCount;

	if(count <= countPerPage)
	{
		m_mediaPostersStartIndex = 0;
		return;
	}

	int max = count-countPerPage;
	m_mediaPostersStartIndex = index<0?0:(index>max?max:index);
	
}
std::string IconMenu::getMediaAtIndexOnScreen(int index)const
{
	juce::File f=getMediaFileAtIndexOnScreen(index);
	return f.exists()?f.getFullPathName().toUTF8().getAddress():std::string();
}
juce::File IconMenu::getMediaFileAtIndexOnScreen(int indexOnScreen)const
{
	return getMediaFileAtIndexInfolder(getFileIndexAtScreenIndex(indexOnScreen));
}
juce::File IconMenu::getMediaFileAtIndexInfolder(int indexInfolder)const
{
	if(indexInfolder >= 0)
	{
		const juce::ScopedLock currentFilesMutexScopedLock (m_currentFilesMutex);
		if( indexInfolder<m_currentFiles.size() )
		{
			return m_currentFiles[indexInfolder].first;
		}
	}

	if(indexInfolder == IconMenu::UpFolderIndex)
	{
		const juce::ScopedLock myScopedLock (m_mutex);
		return juce::File(juce::String::fromUTF8(m_mediaPostersRoot.c_str())).getParentDirectory();
	}
	else
	{
		return juce::File();
	}
}
int IconMenu::getFileIndexAtScreenIndex(int indexOnScreen)const
{
	if(IconMenu::InvalidIndex == indexOnScreen)
	{
		return IconMenu::InvalidIndex;
	}

    const juce::ScopedLock myScopedLock (m_mutex);
	bool hasUpItem = m_mediaPostersAbsoluteRoot != m_mediaPostersRoot;
	if( hasUpItem && indexOnScreen==0)
	{
		return IconMenu::UpFolderIndex;
	}
	
	int indexInfolder = indexOnScreen + m_mediaPostersStartIndex - (hasUpItem?1:0);
		
	if(indexInfolder>=0)
	{
		return indexInfolder;
	}
	return IconMenu::InvalidIndex;
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
	const float sliderRelativePos = 0.97f;
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

int IconMenu::mediaCount() const
{
	int fileCount;
	{
		const juce::ScopedLock myScopedLock (m_currentFilesMutex);
		fileCount = m_currentFiles.size();
	}

	const juce::ScopedLock myScopedLock (m_mutex);
	
	return fileCount + (m_mediaPostersAbsoluteRoot != m_mediaPostersRoot?1:0);
}
void IconMenu::paintMenu(juce::Graphics& g, float w, float h) const
{
	juce::Rectangle<float> sliderRect = computeSliderRect(w, h);
	int countPerPage=m_mediaPostersXCount*m_mediaPostersYCount;
	int count=mediaCount();
	for(int i=0;i<std::min(count,countPerPage);i++)
	{
		paintItem(g,  i,  w, sliderRect.getY());
	}
	

	if(count > countPerPage)
	{
		//draw slider

		int firstMediaIndex=(m_mediaPostersStartIndex + countPerPage) > count ? count - countPerPage: m_mediaPostersStartIndex;
		float sliderStart = firstMediaIndex/(float)count;
		float sliderEnd = (firstMediaIndex+countPerPage)/(float)count;
		float sliderSize = sliderEnd-sliderStart;

		g.setColour(juce::Colours::darkgrey);
		
		//base
		g.fillRect(0.f, sliderRect.getY(), w, sliderRect.getHeight());
	

		//position
		g.setColour(juce::Colours::grey);

		juce::Path rect;
		rect.addRectangle(sliderRect.getX()+sliderRect.getWidth()*sliderStart, sliderRect.getY(), sliderRect.getWidth()*sliderSize, sliderRect.getHeight());
	
		g.fillPath(rect);
		
		g.setColour(juce::Colours::black);

		juce::AffineTransform arrowScale = juce::AffineTransform::scale(0.25f*sliderRect.getX(), 0.25f*sliderRect.getHeight());
		float arrowxMargin = 0.25f*sliderRect.getX();
		juce::Path leftArrow;
		leftArrow.startNewSubPath(1.f, -1.f);
		leftArrow.lineTo(0.f, 0.f);
		leftArrow.lineTo(1.f,  1.f);
		leftArrow.applyTransform(arrowScale.followedBy(juce::AffineTransform::translation(arrowxMargin, sliderRect.getCentreY())));
		
		g.strokePath(leftArrow, juce::PathStrokeType (0.1f*sliderRect.getHeight()));
		
		juce::Path rightArrow;
		rightArrow.startNewSubPath(-1.f, -1.f);
		rightArrow.lineTo(0.f, 0.f);
		rightArrow.lineTo(-1.f,  1.f);
		rightArrow.applyTransform(arrowScale.followedBy(juce::AffineTransform::translation(w-arrowxMargin, sliderRect.getCentreY())));
			
		g.strokePath(rightArrow, juce::PathStrokeType (0.1f*sliderRect.getHeight()));


	}

	if(m_mediaPostersRoot.empty())
	{
		//help
		juce::Font f = g.getCurrentFont().withHeight(sliderRect.getHeight());
		f.setStyleFlags(juce::Font::plain);
		g.setFont(f);
		g.setColour (juce::Colours::white);
		g.drawFittedText(TRANS("You may set this frontpage root folder from the right click file browser menu (end of list)"),
			  (int)(sliderRect.getX()),
			  (int)(sliderRect.getY()-2.f*sliderRect.getHeight()), 
			  (int)sliderRect.getWidth(), 
			  (int)(2.f*sliderRect.getHeight()), 
			juce::Justification::centred, 2);
	}
}

void IconMenu::paintItem(juce::Graphics& g, int indexOnScreen, float w, float h) const
{

	
	const juce::Colour PURPLE(162,0,255);
	const juce::Colour MAGENTA(255, 0, 151);
	const juce::Colour TEAL(0, 171, 169);
	const juce::Colour LIME(140, 191, 38);
	const juce::Colour BROWN(160, 80, 0);
	const juce::Colour PINK(230, 113, 184);
	const juce::Colour ORANGE(240, 150, 9);
	const juce::Colour BLUE(27, 161, 226);
	const juce::Colour RED(229, 20, 0);
	const juce::Colour GREEN(51, 153, 51);

	const juce::Colour colorTable[] = {PURPLE, TEAL, LIME, MAGENTA, BROWN, PINK, ORANGE, BLUE, RED, GREEN};
#define colorTableLen  10
				
	int indexInfolder = getFileIndexAtScreenIndex(indexOnScreen);
	juce::Colour itemColor = colorTable[indexInfolder%colorTableLen];
	
	bool itemSelected = (indexOnScreen == m_mediaPostersHightlight);

	juce::File file=getMediaFileAtIndexInfolder(indexInfolder);
	if(!file.exists())
	{
		return;
	}

	juce::Rectangle<float> rect = getButtonAt(indexOnScreen, w, h);

	float lineThickness = 1.5f;
        
	g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);

	
	
	float titleHeight = rect.getHeight()*0.2f;
	juce::Rectangle<float> titleRect(rect.getX(), rect.getBottom()-titleHeight, rect.getWidth(), titleHeight);
	const int maxTitleLineCount = 3;

	juce::ColourGradient titleGradient(itemColor,
									titleRect.getX(),titleRect.getY(),
									itemSelected ? itemColor.brighter(1.5) : itemColor.brighter(0.5),
									titleRect.getRight()+titleRect.getWidth()/2.f,titleRect.getBottom()+titleRect.getHeight()/2.f,
									true);

	juce::Rectangle<float> imageTargetRect(rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight()-titleHeight);

	
	g.setOpacity (1.f);
	
	if(file.isDirectory())
	{
		float topOffset = titleHeight/8.f;

		int nbStacked = 4;
		float stackItemHeight = topOffset/2.f;
		float stackItemSpacing = topOffset/4.f;
		float aboveBorder = (stackItemHeight+stackItemSpacing) * nbStacked;
		
		g.setGradientFill(titleGradient);

		for(int i= 0;i<nbStacked;++i)
		{
			g.fillRect(rect.getX(), rect.getY() + i*(stackItemHeight+stackItemSpacing), rect.getWidth(), stackItemHeight);
		}
		
		juce::Rectangle<float> borderRect(rect);
		borderRect.translate(0, aboveBorder);
		borderRect.setHeight(borderRect.getHeight()-aboveBorder);

		g.fillRect(borderRect);

		imageTargetRect.translate(topOffset, topOffset+aboveBorder);
		imageTargetRect.setWidth(imageTargetRect.getWidth()-2.f*topOffset);
		imageTargetRect.setHeight(imageTargetRect.getHeight()-2.f*topOffset-aboveBorder);

	}
	
	bool isUpIcon = indexOnScreen == 0 && m_mediaPostersAbsoluteRoot != m_mediaPostersRoot;	
	if(isUpIcon && upImage)
	{
		upImage.get()->drawWithin (g, rect,
							juce::RectanglePlacement::centred, 1.0f);
		return;
	}

	bool isRoot = file.getParentDirectory().getFullPathName() == file.getFullPathName();
	if(isRoot)
	{
		//drive
		juce::Drawable const* image = file.isOnCDRomDrive()?diskImage.get():file.isOnRemovableDrive()?usbImage.get():driveImage.get();
		float s = std::min(rect.getWidth(), rect.getHeight());
		image->drawWithin (g, rect.withSize(s, s),
							juce::RectanglePlacement::xMid | juce::RectanglePlacement::yTop | juce::RectanglePlacement::stretchToFit, 1.0f);
	}
	else
	{
	
		juce::Image image = m_imageCatalog.get(file);
		if(image.isNull() && !file.isDirectory())
		{
			image = appImage;
		}

		if(m_thumbnailer.busyOn(file))
		{
			g.setColour(itemColor.brighter());
			g.fillRect(imageTargetRect);
		}
		

		if(!image.isNull())
		{
			//picture
			float imgHeightRatio = (float)image.getHeight()/(float)image.getWidth();
			float itemHeightRatio = imageTargetRect.getHeight()/imageTargetRect.getWidth();
			float scale;
			float imageX;
			float imageY;
			if(imgHeightRatio > itemHeightRatio)
			{
				//scale on h
				scale = imageTargetRect.getHeight()/(image.getHeight());
				imageX = imageTargetRect.getX() + (imageTargetRect.getWidth()-image.getWidth()*scale)/2;
				imageY = imageTargetRect.getY();
			}
			else
			{
				//scale on w
				scale = imageTargetRect.getWidth()/image.getWidth();
				imageX = imageTargetRect.getX();
				imageY = imageTargetRect.getBottom() - image.getHeight()*scale;
			}
			float imageItemW = image.getWidth()*scale;
			float imageItemH = image.getHeight()*scale;
		
		
			juce::AffineTransform tr = juce::AffineTransform::identity.scaled(scale, scale).translated(imageX, imageY);
			g.drawImageTransformed(image, tr, false);
			
		}
	}

	//title background
	if(!file.isDirectory())
	{
		g.setGradientFill(titleGradient);
		g.fillRect(titleRect);
	}

	//title
	juce::Font f = g.getCurrentFont().withHeight(titleHeight/maxTitleLineCount);
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);
	g.setColour ((isUpIcon)?juce::Colours::white:juce::Colours::black);
	g.drawFittedText(file.exists()?isUpIcon?file.getParentDirectory().getFullPathName():
		(isRoot?(file.getFileName()+juce::String(" (")+file.getVolumeLabel()+juce::String(")")):file.getFileNameWithoutExtension()):juce::String::empty,
		(int)titleRect.getX(),  (int)titleRect.getY(), 
		(int)titleRect.getWidth(), (int)titleRect.getHeight(), 
		juce::Justification::centred, maxTitleLineCount, 1.f);
	
}
bool IconMenu::updateFilePreview(PathAndImage & pathAndImageLoadedFlag)
{
	if(pathAndImageLoadedFlag.second)
	{
		//already processed
		return false;
	}
	juce::File const& f(pathAndImageLoadedFlag.first);

	
	if(f.getParentDirectory().getFullPathName() == f.getFullPathName())
	{
		//root (hd, disk, usb)
		return false;
	}

	juce::File fileToAnalyse;
	fileToAnalyse = f.isDirectory()?findFirstMovie(f):f;
	if(!fileToAnalyse.exists())
	{
		return false;
	}
	if(m_imageCatalog.get(f).isNull())
	{
		//process this one
		juce::Image poster = PosterFinder::findPoster(fileToAnalyse);
		if(!poster.isNull())
		{
			m_imageCatalog.storeImageInCacheAndSetChanged(f, poster);
			pathAndImageLoadedFlag.second = true;
			return true;
		}
		return m_thumbnailer.startGeneration(f, fileToAnalyse);
	}
	//found in cache, update flag
	pathAndImageLoadedFlag.second = true;
	return false;
}
bool IconMenu::updatePreviews()
{
	if(m_thumbnailer.workStep())
	{
		//generation updated, refresh now
		return true;
	}

	
		
	{
		const juce::ScopedLock myScopedLock (m_currentFilesMutex);
		//process visible items first
		int countPerPage=m_mediaPostersXCount*m_mediaPostersYCount;
		int count=mediaCount();
		for(int i=0;i<std::min(count,countPerPage);i++)
		{
			int fileIndex = getFileIndexAtScreenIndex(i);
			if(fileIndex >= 0 && fileIndex < m_currentFiles.size() && updateFilePreview(m_currentFiles.getReference(fileIndex)) )
			{
				return true;
			}
		}
	}

	
	{
		const juce::ScopedLock myScopedLock (m_currentFilesMutex);
		//process all current folder items
		for(PathAndImage* it = m_currentFiles.begin();it != m_currentFiles.end();++it)
		{
			if(updateFilePreview(*it))
			{
				return true;
			}
		}
	}
	//nothing to do
	m_imageCatalog.maySaveCache();
	return false;

}

