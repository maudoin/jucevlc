#ifndef POSTERFINDER_H
#define POSTERFINDER_H


#include "juce.h"

class ImageCatalog;

namespace PosterFinder
{
	juce::Image findPoster(juce::File const& file);
};


#endif //POSTERFINDER_H