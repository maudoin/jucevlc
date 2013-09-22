
#include "PosterFinder.h"
#include "ImageCatalog.h"
#include <boost/format.hpp>
#include <boost/regex.hpp>

namespace PosterFinder
{
juce::Image PosterFinder::findPoster(juce::File const& file)
{
	juce::String movieName = file.getFileNameWithoutExtension();
	if(movieName.isEmpty())
	{
		//all cache already fully setup
		return juce::Image::null;
	}
	movieName = movieName.replace("%", "%37");
	movieName = movieName.replace(" ", "%20");
	movieName = movieName.replace("_", "%20");
	movieName = movieName.replace(".", "%20");
	movieName = movieName.replace("\u00E9", "e");//é
	movieName = movieName.replace("\u00E9", "e");//è
	movieName = movieName.replace("\u00F4", "o");//ô
	movieName = movieName.replace("\u00E0", "a");//à
	std::string name = str( boost::format("http://www.omdbapi.com/?i=&t=%s")%std::string(movieName.toUTF8().getAddress()) );
	juce::URL url(name.c_str());
	juce::ScopedPointer<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 1000, 0));
	if(!pIStream.get())
	{
		return juce::Image::null;
	}
	juce::MemoryOutputStream memStream(1000);//1ko at least
	if(memStream.writeFromInputStream(*pIStream, 100000)<=0)//100ko max
	{
		return juce::Image::null;
	}

	std::string ex("\"Poster\":\"([^\"]*)");
	boost::regex expression(ex, boost::regex::icase); 
	
	memStream.writeByte(0);//simulate end of c string
	boost::cmatch matches; 
	if(!boost::regex_search((char*)memStream.getData(), matches, expression)) 
	{
		return juce::Image::null;
	}
	juce::URL urlPoster(matches[1].str().c_str());
	juce::ScopedPointer<juce::InputStream> pIStreamImage(urlPoster.createInputStream(false, 0, 0, "", 1000, 0));//1 sec timeout
	if(!pIStreamImage.get())
	{
		return juce::Image::null;
	}
	return juce::ImageFileFormat::loadFrom (*pIStreamImage);
}

};