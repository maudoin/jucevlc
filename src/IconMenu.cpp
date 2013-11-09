
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
	:m_mediaPostersXCount(5)
	,m_mediaPostersYCount(2)
	,m_leftArrowHighlighted(false)
	,m_rightArrowHighlighted(false)
	,m_sliderHighlighted(false)
	,m_thumbnailer(m_imageCatalog)
{
    appImage = juce::ImageFileFormat::loadFrom(vlc_png, vlc_pngSize);
    folderBackImage = juce::Drawable::createFromImageData (verticalFolderback_svg, verticalFolderback_svgSize);
    folderFrontImage = juce::Drawable::createFromImageData (verticalFolderfront_svg, verticalFolderfront_svgSize);
    driveImage = juce::Drawable::createFromImageData (harddisk_svg, harddisk_svgSize);
    diskImage = juce::Drawable::createFromImageData (disk_svg, disk_svgSize);
    usbImage = juce::Drawable::createFromImageData (usb_svg, usb_svgSize);
    upImage = juce::Drawable::createFromImageData (back_svg, back_svgSize);

	

}

IconMenu::~IconMenu()
{
}
	
juce::Rectangle<float> IconMenu::getButtonAt(int index, float w, float h)const
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
std::string IconMenu::getMediaAt(float xPos, float yPos, float w, float h)const
{
	return getMediaAt(getButtonIndexAt(xPos, yPos, w, h));
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
std::string IconMenu::getMediaAt(int index)const
{
	juce::File f=getMediaFileAt(index);
	return f.exists()?f.getFullPathName().toUTF8().getAddress():std::string();
}
juce::File IconMenu::getMediaFileAt(int indexOnScreen)const
{
	int indexInfolder = getFileIndexAtScreenIndex(indexOnScreen);
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
		int firstMediaIndex=(m_mediaPostersStartIndex + countPerPage) > count ? count - countPerPage: m_mediaPostersStartIndex;
		float sliderStart = firstMediaIndex/(float)count;
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

void IconMenu::paintItem(juce::Graphics& g, int index, float w, float h) const
{
	juce::File file=getMediaFileAt(index);
	if(!file.exists())
	{
		return;
	}

	juce::Rectangle<float> rect = getButtonAt(index, w, h);
	
	juce::Rectangle<float> firstRect = getButtonAt(0, w, h);
	float spaceX = firstRect.getX();
	float spaceY = firstRect.getY();
	
	float lineThickness = 1.5f;
        
	g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
	
	float sideStripWidth = 2.f*spaceX/3.f;		
	float holeW = 2.f*sideStripWidth/3.f;
	float holeH = holeW;	
	juce::Rectangle<float> rectWithBorders(rect.getX()-sideStripWidth, rect.getY()-holeH, rect.getWidth()+2.f*sideStripWidth, rect.getHeight()+spaceY+2.f*holeH);
	
	float titleHeight = rectWithBorders.getBottom()-rect.getBottom();
	const int maxTitleLineCount = 3;

	if(index == m_mediaPostersHightlight)
	{
		g.setColour(juce::Colours::purple);
		g.fillRect(rectWithBorders);
	}

	g.setOpacity (1.f);

	
	bool isUpIcon = index == 0 && m_mediaPostersAbsoluteRoot != m_mediaPostersRoot;
	if(isUpIcon)
	{
		upImage.get()->drawWithin (g, rect,
							juce::RectanglePlacement::centred | juce::RectanglePlacement::stretchToFit, 1.0f);
		return;
	}
	
	bool isRoot = file.getParentDirectory().getFullPathName() == file.getFullPathName();
	if(isRoot)
	{
		//drive
		juce::Drawable const* image = file.isOnCDRomDrive()?diskImage.get():file.isOnRemovableDrive()?usbImage.get():driveImage.get();
		float s = std::min(rect.getWidth(), rect.getHeight());
		image->drawWithin (g, rect.withSize(s, s),
							juce::RectanglePlacement::centred | juce::RectanglePlacement::stretchToFit, 1.0f);
	}
	else
	{
	
		juce::Image image = m_imageCatalog.get(file);
		if(image.isNull() && !file.isDirectory())
		{
			image = appImage;
		}
	
		juce::Rectangle<float> imageTargetRect(rect);
		if(file.isDirectory())
		{
			folderBackImage.get()->drawWithin (g, rectWithBorders,
								juce::RectanglePlacement::centred | juce::RectanglePlacement::stretchToFit, 1.0f);
		
			//scales are specific to folderImage layout
			float xMarginRelative = 0.02f;
			float yTopMarginRelative = 0.15f;
			float yBottomMarginAbsolute = titleHeight;
			imageTargetRect=rectWithBorders.translated(rectWithBorders.getWidth()*xMarginRelative, rectWithBorders.getHeight()*yTopMarginRelative);
			imageTargetRect.setSize(rectWithBorders.getWidth()*(1.f-2.f*xMarginRelative), rectWithBorders.getHeight()*(1.f-yTopMarginRelative)-yBottomMarginAbsolute);
		}
	
		if(m_thumbnailer.busyOn(file))
		{
			g.setColour(juce::Colours::purple.brighter());
			g.fillRect(imageTargetRect);
		}

		if(!file.isDirectory())
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

			if(!file.isDirectory())
			{

				//reflect
				float reflectionH = titleHeight;
				juce::AffineTransform t = juce::AffineTransform::identity.scaled(scale, -titleHeight/image.getHeight()).translated(imageX, rectWithBorders.getBottom());
				g.drawImageTransformed(image, t, false);

				//reflection floor
				juce::ColourGradient grad (juce::Colours::purple.withAlpha(0.33f),
													imageTargetRect.getX()+imageTargetRect.getWidth()/2.f, imageTargetRect.getBottom(),
													juce::Colours::black.withAlpha(1.0f),
													imageTargetRect.getX()+imageTargetRect.getWidth()/2.f, rectWithBorders.getBottom(),
													false);
		
				grad.addColour(0.66, juce::Colours::purple.withAlpha(.66f));
				g.setGradientFill (grad);	
				g.fillRect(imageTargetRect.getX(), imageTargetRect.getBottom(), imageTargetRect.getWidth(), reflectionH);
	
				//border
				g.setColour(juce::Colour(255, 255, 255));
				g.drawRect(imageTargetRect, lineThickness);
			}
		}

		if(file.isDirectory())
		{
			folderFrontImage.get()->drawWithin (g, rectWithBorders,
								juce::RectanglePlacement::centred | juce::RectanglePlacement::stretchToFit, 1.0f);
		}

	}

	//title
	juce::Font f = g.getCurrentFont().withHeight(titleHeight/maxTitleLineCount);
	f.setStyleFlags(juce::Font::plain);
	g.setFont(f);
	g.setColour (juce::Colours::white);
	g.drawFittedText(file.exists()?isUpIcon?file.getParentDirectory().getFullPathName():
		(isRoot?(file.getFileName()+juce::String(" (")+file.getVolumeLabel()+juce::String(")")):file.getFileNameWithoutExtension()):juce::String::empty,
		(int)(rect.getX()),  (int)rect.getBottom(), 
		(int)(rect.getWidth()), (int)(titleHeight), 
		juce::Justification::centredBottom, maxTitleLineCount, 1.f);

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

