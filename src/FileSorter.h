#ifndef FILESORTER_H
#define FILESORTER_H


#include "juce.h"
#include <set>

bool extensionMatch(std::set<juce::String> const& e, juce::String const& ex);
bool extensionMatch(std::set<juce::String> const& e, juce::File const& f);

struct FileSorter
{
	std::vector< std::set<juce::String> > priorityExtensions;
	FileSorter(std::set<juce::String> const& priorityExtensions_){priorityExtensions.push_back(priorityExtensions_);}
	FileSorter(std::vector< std::set<juce::String> > const& priorityExtensions_):priorityExtensions(priorityExtensions_) {}
	int rank(juce::File const& f);
	int compareElements(juce::File const& some, juce::File const& other);
};



#endif //FILESORTER_H