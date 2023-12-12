
#include "PosterFinder.h"
#include "ImageCatalog.h"
#include <format>
#include <regex>

namespace PosterFinder
{
juce::Image findPoster(juce::File const& file)
{
	juce::String movieName = file.getFileNameWithoutExtension();
	if(movieName.isEmpty())
	{
		//all cache already fully setup
		return {};
	}
	movieName = movieName.replace("%", "%37");
	movieName = movieName.replace(" ", "%20");
	movieName = movieName.replace("_", "%20");
	movieName = movieName.replace(".", "%20");
	movieName = movieName.replace(juce::CharPointer_UTF8("\u00E9"), "e");//é
	movieName = movieName.replace(juce::CharPointer_UTF8("\u00E9"), "e");//è
	movieName = movieName.replace(juce::CharPointer_UTF8("\u00F4"), "o");//ô
	movieName = movieName.replace(juce::CharPointer_UTF8("\u00E0"), "a");//à
	std::string name = std::format("http://www.omdbapi.com/?i=&t={}",std::string(movieName.toUTF8().getAddress()) );
	juce::URL url(juce::CharPointer_UTF8(name.c_str()));
	std::unique_ptr<juce::InputStream> pIStream(url.createInputStream(false, 0, 0, "", 1000, 0));
	if(!pIStream.get())
	{
		return {};
	}
	juce::MemoryOutputStream memStream(1000);//1ko at least
	if(memStream.writeFromInputStream(*pIStream, 100000)<=0)//100ko max
	{
		return {};
	}

	std::string ex("\"Poster\":\"([^\"]*)");
	std::regex expression(ex, std::regex::icase); 
	
	memStream.writeByte(0);//simulate end of c string
	std::cmatch matches; 
	if(!std::regex_search((char*)memStream.getData(), matches, expression)) 
	{
		return {};
	}
	juce::URL urlPoster(matches[1].str().c_str());
	std::unique_ptr<juce::InputStream> pIStreamImage(urlPoster.createInputStream(false, 0, 0, "", 1000, 0));//1 sec timeout
	if(!pIStreamImage.get())
	{
		return {};
	}
	return juce::ImageFileFormat::loadFrom (*pIStreamImage);
}

};